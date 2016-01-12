#include "e_mod_main.h"

E_API E_Module_Api e_modapi = { E_MODULE_API_VERSION, "Quickpanel-Manager" };

E_API void *
e_modapi_init(E_Module *m)
{
   printf("LOAD %s MODULE\n", e_modapi.name);

   e_mod_qpmgr_quickpanel_server_init();
   e_mod_qpmgr_indicator_init();
   e_mod_qpmgr_quickpanel_init();

   return m;
}

E_API int
e_modapi_shutdown(E_Module *m EINA_UNUSED)
{
   e_mod_qpmgr_quickpanel_server_shutdown();
   e_mod_qpmgr_indicator_shutdown();
   e_mod_qpmgr_quickpanel_shutdown();

   return 1;
}

E_API int
e_modapi_save(E_Module *m EINA_UNUSED)
{
   /* Save something to be kept */
   return 1;
}
