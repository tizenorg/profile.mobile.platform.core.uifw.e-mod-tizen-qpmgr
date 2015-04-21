#ifndef E_MOD_MAIN_H
#define E_MOD_MAIN_H

#include <e.h>

/*** E Module ***/
EAPI extern E_Module_Api e_modapi;

EAPI void *e_modapi_init     (E_Module *m);
EAPI int   e_modapi_shutdown (E_Module *m);
EAPI int   e_modapi_save     (E_Module *m);

EINTERN void             e_mod_qpmgr_indicator_init(void);
EINTERN void             e_mod_qpmgr_indicator_shutdown(void);
EINTERN void             e_mod_qpmgr_quickpanel_init(void);
EINTERN void             e_mod_qpmgr_quickpanel_shutdown(void);
EINTERN Evas_Object     *e_mod_qpmgr_quickpanel_object_add(E_Client *ec);
EINTERN Eina_Bool        e_mod_qpmgr_quickpanel_object_handler_move(Evas_Object *obj, int x, int y);
EINTERN E_Client        *e_mod_qpmgr_quickpanel_client_find(void);

#endif
