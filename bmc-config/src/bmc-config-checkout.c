#if HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>
#include <stdlib.h>
#if STDC_HEADERS
#include <string.h>
#endif /* STDC_HEADERS */
#include <errno.h>
#include <assert.h>

#include "bmc-config.h"
#include "bmc-config-checkout.h"
#include "bmc-config-common.h"
#include "bmc-config-sections.h"

static config_err_t
config_checkout_keypair (struct config_section *sections,
                         struct config_keypair *kp,
                         void *arg)
{
  struct config_section *section;
  struct config_keyvalue *kv;
  config_err_t rv = CONFIG_ERR_FATAL_ERROR;
  config_err_t ret = CONFIG_ERR_SUCCESS;

  /* XXX - use common func later */
  section = sections;
  while (section) 
    {
      if (same (kp->section_name, section->section_name)) 
        break;
      section = section->next;
    }
  
  /* XXX check earlier */
  if (!section) 
    {
      fprintf (stderr, "Unknown section `%s'\n", kp->section_name);
      rv = CONFIG_ERR_NON_FATAL_ERROR;
      goto cleanup;
    }
  
  kv = section->keyvalues;
  while (kv) 
    {
      if (same (kp->key_name, kv->key_name))
        break;
      kv = kv->next;
    }

  if (!kv) 
    {
      fprintf (stderr, "Unknown key `%s' in section `%s'\n",
               kp->key_name, kp->section_name);
      rv = CONFIG_ERR_NON_FATAL_ERROR;
      goto cleanup;
    }

  if ((ret = kv->checkout (section, kv, arg)) == CONFIG_ERR_FATAL_ERROR)
    goto cleanup;

  if (ret == CONFIG_ERR_SUCCESS) 
    printf ("%s:%s=%s\n", kp->section_name, kp->key_name, kv->value);
  else
    printf ("Error fetching value for %s in %s\n",
            kp->key_name, kp->section_name);

  rv = ret;
 cleanup:
  return rv;
}

static config_err_t
config_checkout_keypairs (struct config_section *sections,
                          struct config_arguments *cmd_args,
                          void *arg)
{
  struct config_keypair *kp;
  config_err_t rv = CONFIG_ERR_FATAL_ERROR;
  config_err_t ret = CONFIG_ERR_SUCCESS;

  kp = cmd_args->keypairs;
  while (kp)
    {
      config_err_t this_ret;

      if ((this_ret = config_checkout_keypair(sections,
                                              kp,
                                              arg)) == CONFIG_ERR_FATAL_ERROR)
        goto cleanup;
      
      if (this_ret == CONFIG_ERR_NON_FATAL_ERROR)
        ret = CONFIG_ERR_NON_FATAL_ERROR;

      kp = kp->next;
    }

  rv = ret;
 cleanup:
  return rv;
}

static config_err_t
config_checkout_section_common (struct config_section *section, 
                                struct config_arguments *cmd_args,
                                FILE *fp,
                                void *arg)
{
  struct config_keyvalue *kv;
  config_err_t rv = CONFIG_ERR_FATAL_ERROR;
  config_err_t ret = CONFIG_ERR_SUCCESS;
  config_err_t this_ret;

  if (section->flags & CONFIG_DO_NOT_CHECKOUT)
    return ret;

  if (section->section_comment_section_name
      && section->section_comment)
    {
      if (config_section_comments(section->section_comment_section_name,
                                  section->section_comment,
                                  fp) < 0)
        {
          if (cmd_args->verbose)
            fprintf (fp, "\t## FATAL: Comment output error\n");
          ret = CONFIG_ERR_NON_FATAL_ERROR;
        }
    }

  fprintf (fp, "Section %s\n", section->section_name);

  kv = section->keyvalues;
  
  while (kv) 
    { 
      /* achu: Certain keys should be "hidden" and not be checked out.
       * They only linger for backwards compatability to FreeIPMI's
       * 0.2.0 bmc-config, which have several options with typoed
       * names.
       */
      if (kv->flags & CONFIG_DO_NOT_CHECKOUT)
        {
          kv = kv->next;
          continue;
        }

      if ((this_ret = kv->checkout (section,
                                    kv,
                                    arg)) == CONFIG_ERR_FATAL_ERROR)
        goto cleanup;
      
      if (this_ret == CONFIG_ERR_NON_FATAL_ERROR)
        {
          if (cmd_args->verbose)
            fprintf (fp, "\t## FATAL: Unable to checkout %s:%s\n",
                     section->section_name,
                     kv->key_name);
          ret = CONFIG_ERR_NON_FATAL_ERROR;
        } 
      else 
        {
          int key_len = 0;
                  
          fprintf (fp, "\t## %s\n", kv->description);
                  
          /* beauty comes at a cost */
          
          /* achu: Certain keys should have their checked out
           * value automatically commented out.  Sometimes (in the
           * case of passwords) they cannot be checked out, so the
           * default is for value to be empty.  We do not want the
           * user accidently commiting this checked out file,
           * which (in this example) clears the password.
           *
           * Some other keys may or may not have a value, depending on
           * the IPMI version or the implementation.
           */
          if (kv->flags & CONFIG_CHECKOUT_KEY_COMMENTED_OUT)
            key_len = fprintf(fp, "\t## %s", kv->key_name);
          else if (kv->flags & CONFIG_CHECKOUT_KEY_COMMENTED_OUT_IF_VALUE_EMPTY)
            {
              if (kv->value && strlen(kv->value))
                key_len = fprintf (fp, "\t%s", kv->key_name);
              else
                key_len = fprintf(fp, "\t## %s", kv->key_name);
            }
          else
            key_len = fprintf (fp, "\t%s", kv->key_name);
          
          while (key_len <= CONFIG_CHECKOUT_LINE_LEN) 
            {
              fprintf (fp, " ");
              key_len++;
            }
          
          fprintf (fp, "%s\n", kv->value);
        }
      
      kv = kv->next;
    }

  fprintf (fp, "EndSection\n");
  return ret;

 cleanup:
  return rv;
}

static config_err_t
config_checkout_section (struct config_section *sections,
                         struct config_arguments *cmd_args,
                         void *arg)
{
  struct config_section_str *sstr;
  FILE *fp;
  int file_opened = 0;
  config_err_t rv = CONFIG_ERR_FATAL_ERROR;
  config_err_t ret = CONFIG_ERR_SUCCESS;

  if (cmd_args->filename && strcmp (cmd_args->filename, "-"))
    {
      if (!(fp = fopen (cmd_args->filename, "w")))
        {
          perror("fopen");
          goto cleanup;
        }
      file_opened++;
    }
  else
    fp = stdout;

  sstr = cmd_args->section_strs;
  while (sstr)
    {
      struct config_section *section = sections;
      int found = 0;
      
      while (section) 
        {
          if (!strcasecmp(section->section_name, sstr->section_name))
            {
              config_err_t this_ret;
              
              found++;

              if ((this_ret = config_checkout_section_common (section, 
                                                              cmd_args,
                                                              fp,
                                                              arg)) == CONFIG_ERR_FATAL_ERROR)
                goto cleanup;
              
              if (this_ret == CONFIG_ERR_NON_FATAL_ERROR)
                ret = CONFIG_ERR_NON_FATAL_ERROR;
              
              break;
            }

          section = section->next;
        }

      if (!found)
        {
          fprintf(stderr, "Invalid section `%s'\n", sstr->section_name);
          ret = 0;
        } 

      sstr = sstr->next;
    }
  
  rv = ret;
 cleanup:
  if (file_opened)
    fclose(fp);
  return rv;
}

static config_err_t
config_checkout_file (struct config_section *sections,
                      struct config_arguments *cmd_args,
                      void *arg)
{
  struct config_section *section;
  FILE *fp;
  int file_opened = 0;
  config_err_t rv = CONFIG_ERR_FATAL_ERROR;
  config_err_t ret = CONFIG_ERR_SUCCESS;

  if (cmd_args->filename && strcmp (cmd_args->filename, "-"))
    {
      if (!(fp = fopen (cmd_args->filename, "w")))
        {
          perror("fopen");
          goto cleanup;
        }
      file_opened++;
    }
  else
    fp = stdout;

  section = sections;
  while (section) 
    {
      config_err_t this_ret;
      
      if ((this_ret = config_checkout_section_common (section,
                                                      cmd_args,
                                                      fp,
                                                      arg)) == CONFIG_ERR_FATAL_ERROR)
        goto cleanup;

      if (this_ret == CONFIG_ERR_NON_FATAL_ERROR)
        ret = CONFIG_ERR_NON_FATAL_ERROR;

      if (cmd_args->verbose)
        fprintf (stderr, "Completed checkout of Section: %s\n",
                 section->section_name);

      section = section->next;
    }

  rv = ret;
 cleanup:
  if (file_opened)
    fclose(fp);
  return rv;
}

config_err_t
config_checkout (struct config_section *sections,
                 struct config_arguments *cmd_args,
                 void *arg)
{
  config_err_t ret;

  if (cmd_args->keypairs) 
    ret = config_checkout_keypairs (sections, cmd_args, arg);
  else if (cmd_args->section_strs)
    ret = config_checkout_section (sections, cmd_args, arg);
  else
    ret = config_checkout_file (sections, cmd_args, arg);

  return ret;
}
