#include "e_mod_main.h"

typedef struct _Pol_Indicator Pol_Indicator;

struct _Pol_Indicator
{
   E_Client *ec;
   // indicator state, mode
   Eina_Bool clicked : 1;
   Eina_Bool state: 1; // Indicator state on or off
};

static Eina_List *indicator_handlers = NULL;
static Eina_List *indicator_hooks = NULL;
static Eina_Hash *hash_pol_indicators = NULL;

// Quickpanel object
static Evas_Object *quickpanel = NULL;
static Evas_Object *quickpanel_clipper = NULL;
static int quickpanel_x = 0;
static int quickpanel_y = 0;

static void
_pol_cb_clients_data_free(void *data)
{
  free(data);
}

static void
_indicator_evas_cb_mouse_move(void *data, Evas *evas EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event)
{
   E_Client *ec;
   Evas_Event_Mouse_Move *ev;
   Eina_List *l;
   Pol_Indicator *pi;

   ev = event;

   if (!quickpanel) return;

   if (!(ec = data)) return;
   //if (ec->cur_mouse_action) return;
   if (e_object_is_del(E_OBJECT(ec))) return;
   if (e_client_util_ignored_get(ec)) return;

   pi = eina_hash_find(hash_pol_indicators, &ec);
   if ((!pi) || (!pi->clicked)) return;

   e_mod_qpmgr_quickpanel_object_handler_move(quickpanel, 0, ev->cur.canvas.y);
}

static void
_indicator_evas_cb_mouse_down(void *data, Evas *evas EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event)
{
   E_Client *ec = data;
   Evas_Event_Mouse_Down *ev = event;
   //check pressed position, find quickpanel, add mirror object
   Pol_Indicator *pi;
   E_Client *qp_ec = NULL;

   pi = eina_hash_find(hash_pol_indicators, &ec);
   if (!pi) return;

   // Check Indicator region
   if (!E_INSIDE(ev->canvas.x - ec->client.x , ev->canvas.y - ec->client.y, 0, 0, ec->w, 100)) return;

   qp_ec = e_mod_qpmgr_quickpanel_client_find();
   if (!qp_ec) return;

   pi->clicked = EINA_TRUE;

   quickpanel = e_mod_qpmgr_quickpanel_object_add(qp_ec); //find quickpanel's ec
   evas_object_move(quickpanel, 0, 0); // 0 angle case
   evas_object_resize(quickpanel, qp_ec->w, qp_ec->h);
   evas_object_show(quickpanel);
   e_mod_qpmgr_quickpanel_object_handler_move(quickpanel, 0, ev->canvas.y - ec->y);
}

static void
_indicator_evas_cb_mouse_up(void *data, Evas *evas EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event)
{
   E_Client *ec = data;
   Evas_Event_Mouse_Up *ev = event;

   // free mirror object
   Pol_Indicator *pi;
   E_Client *qp_ec = NULL;

   pi = eina_hash_find(hash_pol_indicators, &ec);
   if ((!pi) || (!pi->clicked)) return;

   qp_ec = e_mod_qpmgr_quickpanel_client_find();

   pi->clicked = EINA_FALSE;

   if (ev->canvas.y > ec->zone->h / 2)
     evas_object_move(qp_ec->frame, 0, 0);
   else
     evas_object_move(qp_ec->frame, -10000, -10000);

   evas_object_hide(quickpanel);
   evas_object_del(quickpanel);
   quickpanel = NULL;

}

static void
_indicator_client_evas_init(E_Client *ec)
{
   Pol_Indicator *pi;

   if (eina_hash_find(hash_pol_indicators, &ec)) return;

   /* setup input callbacks */
   evas_object_event_callback_add(ec->frame, EVAS_CALLBACK_MOUSE_MOVE,
                                  _indicator_evas_cb_mouse_move, ec);
   evas_object_event_callback_add(ec->frame, EVAS_CALLBACK_MOUSE_DOWN,
                                  _indicator_evas_cb_mouse_down, ec);
   evas_object_event_callback_add(ec->frame, EVAS_CALLBACK_MOUSE_UP,
                                  _indicator_evas_cb_mouse_up, ec);

   pi = E_NEW(Pol_Indicator, 1);
   pi->ec = ec;

   eina_hash_add(hash_pol_indicators, &ec, pi);
}

static Eina_Bool
_indicator_cb_comp_object_add(void *data EINA_UNUSED, int type EINA_UNUSED, E_Event_Comp_Object *ev)
{
   E_Client *ec;

   /* try to get the client from the object */
   if (!(ec = e_comp_object_client_get(ev->comp_object)))
     return ECORE_CALLBACK_RENEW;

   /* check for client being deleted */
   if (e_object_is_del(E_OBJECT(ec))) return ECORE_CALLBACK_RENEW;

   /* check for wayland pixmap */
   if (e_pixmap_type_get(ec->pixmap) != E_PIXMAP_TYPE_WL) return ECORE_CALLBACK_RENEW;

   if (!e_util_strcmp(ec->icccm.title, "QUICKPANEL")) return ECORE_CALLBACK_RENEW;

   /* if we have not setup evas callbacks for this client, do it */
   if (eina_hash_find(hash_pol_indicators, &ev->comp_object)) return ECORE_CALLBACK_RENEW;

   _indicator_client_evas_init(ec);

   return ECORE_CALLBACK_RENEW;
}

static void
_indicator_hook_client_del(void *d EINA_UNUSED, E_Client *ec)
{
    if (!ec) return;
    if (eina_hash_find(hash_pol_indicators, &ec))
      eina_hash_del_by_key(hash_pol_indicators, &ec);
}

#undef E_CLIENT_HOOK_APPEND
#define E_CLIENT_HOOK_APPEND(l, t, cb, d) \
  do                                      \
    {                                     \
       E_Client_Hook *_h;                 \
       _h = e_client_hook_add(t, cb, d);  \
       assert(_h);                        \
       l = eina_list_append(l, _h);       \
    }                                     \
  while (0)

EINTERN void
e_mod_qpmgr_indicator_init(void)
{
   hash_pol_indicators = eina_hash_pointer_new(_pol_cb_clients_data_free);

   E_LIST_HANDLER_APPEND(indicator_handlers, E_EVENT_COMP_OBJECT_ADD,
                         _indicator_cb_comp_object_add, NULL);
   E_CLIENT_HOOK_APPEND(indicator_hooks, E_CLIENT_HOOK_DEL,
                     _indicator_hook_client_del, NULL);
}

EINTERN void
e_mod_qpmgr_indicator_shutdown(void)
{
   E_FREE_LIST(indicator_hooks, e_client_hook_del);
   E_FREE_LIST(indicator_handlers, ecore_event_handler_del);
   E_FREE_FUNC(hash_pol_indicators, eina_hash_free);
}
