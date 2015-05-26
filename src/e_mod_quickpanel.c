#include "e_mod_main.h"

#define SMART_NAME "quickpanel_object"
/* for non-util functions */
#define API_ENTRY     Quickpanel_Data *qp_data; \
                      qp_data = evas_object_smart_data_get(obj); \
                      if ((!obj) || (!qp_data) || (e_util_strcmp(evas_object_type_get(obj), SMART_NAME))) return

/* for util functions (obj may or may not be Quickpanel_Object */
#define SOFT_ENTRY(...)      Quickpanel_Data *qp_data; \
                       if (!obj) \
                         { \
                            CRI("YOU PASSED NULL!"); \
                            return __VA_ARGS__; \
                         } \
                       qp_data = evas_object_smart_data_get(obj); \
                       if ((!qp_data) || (e_util_strcmp(evas_object_type_get(obj), SMART_NAME))) \
                         qp_data = NULL

#define INTERNAL_ENTRY Quickpanel_Data *qp_data; qp_data = evas_object_smart_data_get(obj);

//Quickpanel Policy Data
typedef struct _Pol_Quickpanel Pol_Quickpanel;

struct _Pol_Quickpanel
{
   E_Client *ec;
   Eina_Bool clicked : 1;

   struct
   {
      int x;
      int y;
      int w;
      int h;
   } handler_geo; // handler geometry
};

static Pol_Quickpanel *_pol_quickpanel = NULL;

static Eina_List *quickpanel_handlers = NULL;
static Eina_List *quickpanel_hooks = NULL;

//Quickpanel object
static Evas_Object *quickpanel = NULL;

typedef struct _Quickpanel_Data
{
   EINA_INLIST;
   E_Client    *ec; // quickpanel's e_client

   Evas_Object *smart_obj; //smart object
   Evas_Object *qp_layout_obj; // quickpanel's e_layout_object
   Evas_Object *base_mirror_obj; // quickpanel base mirror object
   Evas_Object *handler_mirror_obj; // quickpanel handler mirror object
   Evas_Object *base_clip; // clipper for quickapnel base object
   Evas_Object *handler_clip; // clipper for quickpanel handler object

   Eina_Bool    qp_layout_init: 1;

   struct
   {
      int x;
      int y;
      int w;
      int h;
   } handler_geo; // handler geometry

   int angle;

   struct
   {
      Eina_Bool       animating : 1;
      int             sx; // start x postion
      int             sy; // start y position
      int             ex; // end x position
      int             ey; // end y position
      int             dx; // x distance
      int             dy; // y distance
      Ecore_Animator *anim;
   } animation_info; // show or hide animation info
} Quickpanel_Data;

static Evas_Smart *_quickpanel_smart = NULL;

static void
_quickpanel_object_intercept_show(void *data, Evas_Object *obj EINA_UNUSED)
{
   Quickpanel_Data *qp_data = data;
   E_Client *ec = qp_data->ec;

   if (qp_data->qp_layout_init)
     {
        evas_object_show(qp_data->smart_obj);
        return;
     }
   else
     {
        evas_object_color_set(qp_data->ec->frame, 0, 0, 0, 0);

        qp_data->base_mirror_obj =  e_comp_object_util_mirror_add(ec->frame);
        e_layout_pack(qp_data->qp_layout_obj, qp_data->base_mirror_obj);
        e_layout_child_move(qp_data->base_mirror_obj, 0, 0);
        e_layout_child_resize(qp_data->base_mirror_obj, ec->w, ec->h);
        evas_object_show(qp_data->base_mirror_obj);

        qp_data->base_clip = evas_object_rectangle_add(e_comp->evas);
        e_layout_pack(qp_data->qp_layout_obj, qp_data->base_clip);
        e_layout_child_move(qp_data->base_clip, 0, 0);
        e_layout_child_resize(qp_data->base_clip, ec->w, ec->h);
        evas_object_color_set(qp_data->base_clip, 255, 255, 255, 255);
        evas_object_show(qp_data->base_clip);

        evas_object_clip_set(qp_data->base_mirror_obj, qp_data->base_clip);

        // for handler object
        qp_data->handler_mirror_obj =  e_comp_object_util_mirror_add(ec->frame);
        e_layout_pack(qp_data->qp_layout_obj, qp_data->handler_mirror_obj);
        e_layout_child_move(qp_data->handler_mirror_obj, qp_data->handler_geo.x, qp_data->handler_geo.y);
        e_layout_child_resize(qp_data->handler_mirror_obj, ec->w, ec->h);
        evas_object_show(qp_data->handler_mirror_obj);

        qp_data->handler_clip = evas_object_rectangle_add(e_comp->evas);
        e_layout_pack(qp_data->qp_layout_obj, qp_data->handler_clip);
        e_layout_pack(qp_data->qp_layout_obj, qp_data->handler_clip);
        e_layout_child_move(qp_data->handler_clip, qp_data->handler_geo.x, qp_data->handler_geo.y);
        e_layout_child_resize(qp_data->handler_clip, qp_data->handler_geo.w, qp_data->handler_geo.h);
        evas_object_color_set(qp_data->handler_clip, 255, 255, 255, 255);
        evas_object_show(qp_data->handler_clip);

        evas_object_clip_set(qp_data->handler_mirror_obj, qp_data->handler_clip);

        qp_data->qp_layout_init = EINA_TRUE;
     }

   evas_object_show(qp_data->smart_obj);
}

static void
_quickpanel_smart_add(Evas_Object *obj)
{
   Quickpanel_Data *qp_data;
   qp_data = E_NEW(Quickpanel_Data, 1);
   qp_data->smart_obj = obj;
   qp_data->qp_layout_obj = e_layout_add(e_comp->evas);
   evas_object_color_set(qp_data->qp_layout_obj, 255, 255, 255, 255);
   evas_object_smart_member_add(qp_data->qp_layout_obj, qp_data->smart_obj);

   evas_object_layer_set(qp_data->smart_obj, EVAS_LAYER_MAX - 1); // EVAS_LAYER_MAX :L cursor layer

   evas_object_smart_data_set(obj, qp_data);

   evas_object_data_set(obj, "qp_obj", qp_data);
   evas_object_move(obj, -1 , -1);

   evas_object_intercept_show_callback_add(obj, _quickpanel_object_intercept_show, qp_data);

}

static void
_quickpanel_smart_del(Evas_Object *obj)
{
   E_Client *ec;

   INTERNAL_ENTRY;

   ec = qp_data->ec;
   qp_data->animation_info.animating = 0;
   if (qp_data->base_clip)
     {
        evas_object_clip_unset(qp_data->base_clip);
        e_layout_unpack(qp_data->base_clip);
        evas_object_del(qp_data->base_clip);
     }
   if (qp_data->handler_clip)
     {
        evas_object_clip_unset(qp_data->handler_clip);
        e_layout_unpack(qp_data->handler_clip);
        evas_object_del(qp_data->handler_clip);
     }
   if (qp_data->base_mirror_obj)
     {
        e_layout_unpack(qp_data->base_mirror_obj);
        evas_object_del(qp_data->base_mirror_obj);
     }
   if (qp_data->handler_mirror_obj)
     {
        e_layout_unpack(qp_data->handler_mirror_obj);
        evas_object_del(qp_data->handler_mirror_obj);
     }

   if (qp_data->qp_layout_obj) evas_object_del(qp_data->qp_layout_obj);

   evas_object_color_set(ec->frame, ec->netwm.opacity, ec->netwm.opacity, ec->netwm.opacity, ec->netwm.opacity);

   //e_object_unref(E_OBJECT(ec)); 
   free(qp_data);
}

static void
_quickpanel_smart_show(Evas_Object *obj)
{
   INTERNAL_ENTRY;

   evas_object_show(qp_data->qp_layout_obj);
}

static void
_quickpanel_smart_hide(Evas_Object *obj)
{
   INTERNAL_ENTRY;
   
   evas_object_hide(qp_data->qp_layout_obj);
}

static void
_quickpanel_smart_move(Evas_Object *obj, int x, int y)
{
   INTERNAL_ENTRY;

   evas_object_move(qp_data->qp_layout_obj, x, y);
}

static void
_quickpanel_smart_resize(Evas_Object *obj, int w, int h)
{
   INTERNAL_ENTRY;

   e_layout_virtual_size_set(qp_data->qp_layout_obj, w, h);
   evas_object_resize(qp_data->qp_layout_obj, w, h);
}

static void
_quickpanel_smart_init(void)
{
   if (_quickpanel_smart) return;
   {
      static const Evas_Smart_Class sc =
      {
         SMART_NAME,
         EVAS_SMART_CLASS_VERSION,
         _quickpanel_smart_add,
         _quickpanel_smart_del,
         _quickpanel_smart_move,
         _quickpanel_smart_resize,
         _quickpanel_smart_show,
         _quickpanel_smart_hide,
         NULL, /* color_set */
         NULL, /* clip_set */
         NULL, /* clip_unset */
         NULL, /* calculate */
         NULL, /* member_add */
         NULL, /* member_del */

         NULL, /* parent */
         NULL, /* callbacks */
         NULL, /* interfaces */
         NULL  /* data */
      };
      _quickpanel_smart = evas_smart_class_new(&sc);
   }
}

EINTERN Evas_Object *
e_mod_qpmgr_quickpanel_object_add(E_Client *ec)
{
   Evas_Object *o;
   Quickpanel_Data *qp_data;

   EINA_SAFETY_ON_NULL_RETURN_VAL(ec, NULL);

   if (!ec->frame) return NULL; // if quickpanel's frame object is not exist then return;

   _quickpanel_smart_init();
   o = evas_object_smart_add(e_comp->evas, _quickpanel_smart);

   qp_data = evas_object_smart_data_get(o);
   evas_object_data_set(o, "E_Client", ec);
   qp_data->ec = ec;

   //TODO: Handler geometry for various angle
   qp_data->handler_geo.w = ec->w;
   qp_data->handler_geo.h = 50;

   //e_object_ref(E_OBJECT(ec)); 
   return o;
}

EINTERN Eina_Bool
e_mod_qpmgr_quickpanel_object_handler_move(Evas_Object *obj, int x, int y)
{
   Quickpanel_Data *qp_data;

   if (!obj) return EINA_FALSE;

   if (!evas_object_data_get(obj, "qp_obj")) return EINA_FALSE;

   qp_data = evas_object_smart_data_get(obj);
   // angle 0 case
   if ((y + qp_data->handler_geo.h) > qp_data->ec->zone->h ) return EINA_FALSE;

   // angle 0 case
   qp_data->handler_geo.y = y ;

   e_layout_child_resize(qp_data->base_clip, qp_data->ec->w, qp_data->handler_geo.y); // base clip resize

   e_layout_child_move(qp_data->handler_mirror_obj, qp_data->handler_geo.x, qp_data->handler_geo.y - qp_data->ec->h + qp_data->handler_geo.h); // handler mirror object move
   e_layout_child_move(qp_data->handler_clip, qp_data->handler_geo.x, qp_data->handler_geo.y); // handler mirror object move

   return EINA_TRUE;
}

EINTERN E_Client *
e_mod_qpmgr_quickpanel_client_find(void)
{
   E_Client *ec;

   E_CLIENT_REVERSE_FOREACH(e_comp, ec)
     {
        if (e_object_is_del(E_OBJECT(ec))) continue;
        if (e_client_util_ignored_get(ec)) continue;
        if (!ec->frame) continue;
        if (!evas_object_visible_get(ec->frame)) continue;

        if (!e_util_strcmp(ec->icccm.title, "QUICKPANEL"))
          break;
     }
   return ec;
}

// QUICKPANEL Policy
static void
_quickpanel_client_evas_cb_mouse_move(void *data, Evas *evas EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event)
{
   E_Client *ec;
   Evas_Event_Mouse_Move *ev = event;

   if (!(ec = data)) return;
   //if (ec->cur_mouse_action) return;
   if (e_object_is_del(E_OBJECT(ec))) return;
   if (e_client_util_ignored_get(ec)) return;

   if ((!_pol_quickpanel) || (!_pol_quickpanel->clicked) || (!quickpanel)) return;

   e_mod_qpmgr_quickpanel_object_handler_move(quickpanel, 0, ev->cur.canvas.y);
}

static void
_quickpanel_client_evas_cb_mouse_down(void *data, Evas *evas EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event)
{
   E_Client *ec = data;
   Evas_Event_Mouse_Down *ev = event;
   int hx, hy, hw, hh;
   //check pressed position, find quickpanel, add mirror object

   if ((!_pol_quickpanel) || (_pol_quickpanel->clicked)) return;

   // Check Indicator region
   hx = _pol_quickpanel->handler_geo.x;
   hy = _pol_quickpanel->handler_geo.y;
   hw = _pol_quickpanel->handler_geo.w;
   hh = _pol_quickpanel->handler_geo.h;

   if (!E_INSIDE(ev->canvas.x - ec->client.x , ev->canvas.y - ec->client.y, hx, hy, hw, hh)) return;

   _pol_quickpanel->clicked = EINA_TRUE;

   quickpanel = e_mod_qpmgr_quickpanel_object_add(ec); //find quickpanel's ec
   evas_object_move(quickpanel, 0, 0); // 0 angle case
   evas_object_resize(quickpanel, ec->w, ec->h);
   evas_object_show(quickpanel);
   e_mod_qpmgr_quickpanel_object_handler_move(quickpanel, 0, ev->canvas.y - ec->y);
}

static void
_quickpanel_client_evas_cb_mouse_up(void *data, Evas *evas EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event)
{
   E_Client *ec = data;
   Evas_Event_Mouse_Up *ev = event;

   if ((!_pol_quickpanel) || (!_pol_quickpanel->clicked)) return;

   _pol_quickpanel->clicked = EINA_FALSE;

   if (ev->canvas.y > ec->zone->h / 2)
     evas_object_move(ec->frame, 0, 0);
   else
     evas_object_move(ec->frame, -10000, -10000);

   if (!quickpanel) return;

   evas_object_hide(quickpanel);
   evas_object_del(quickpanel);
   quickpanel = NULL;
}

static void
_quickpanel_client_evas_init(E_Client *ec)
{
   Pol_Quickpanel *pol_qp = NULL;

   if (_pol_quickpanel) return;

   pol_qp = E_NEW(Pol_Quickpanel, 1);
   if (!pol_qp) return;

   pol_qp->ec = ec;
   // TODO: make handler geometry configurable.
   pol_qp->handler_geo.x = 0;
   pol_qp->handler_geo.w = ec->zone->w;
   pol_qp->handler_geo.h = 100;
   pol_qp->handler_geo.y = ec->zone->h - pol_qp->handler_geo.h;

   /* setup input callbacks */
   evas_object_event_callback_add(ec->frame, EVAS_CALLBACK_MOUSE_MOVE,
                                  _quickpanel_client_evas_cb_mouse_move, ec);
   evas_object_event_callback_add(ec->frame, EVAS_CALLBACK_MOUSE_DOWN,
                                  _quickpanel_client_evas_cb_mouse_down, ec);
   evas_object_event_callback_add(ec->frame, EVAS_CALLBACK_MOUSE_UP,
                                  _quickpanel_client_evas_cb_mouse_up, ec);

   _pol_quickpanel = pol_qp;
}


static Eina_Bool
_quickpanel_cb_comp_object_add(void *data EINA_UNUSED, int type EINA_UNUSED, E_Event_Comp_Object *ev)
{
   E_Client *ec;

   /* try to get the client from the object */
   if (!(ec = e_comp_object_client_get(ev->comp_object)))
     return ECORE_CALLBACK_RENEW;

   /* check for client being deleted */
   if (e_object_is_del(E_OBJECT(ec))) return ECORE_CALLBACK_RENEW;

   /* check for wayland pixmap */
   if (e_pixmap_type_get(ec->pixmap) != E_PIXMAP_TYPE_WL) return ECORE_CALLBACK_RENEW;

   if (e_util_strcmp(ec->icccm.title, "QUICKPANEL") != 0) return ECORE_CALLBACK_RENEW;

   /* if we have not setup evas callbacks for this client, do it */
   if (_pol_quickpanel) return ECORE_CALLBACK_RENEW;

   _quickpanel_client_evas_init(ec);

   return ECORE_CALLBACK_RENEW;
}

static void
_quickpanel_hook_client_del(void *d EINA_UNUSED, E_Client *ec)
{
    if (!ec) return;
    if (!_pol_quickpanel) return;

    if (_pol_quickpanel->ec == ec)
      {
         if (quickpanel)
           {
              evas_object_hide(quickpanel);
              evas_object_del(quickpanel);
              quickpanel = NULL;
           }
         E_FREE(_pol_quickpanel);
         _pol_quickpanel = NULL;
      }
}

EINTERN Eina_Bool
e_mod_qpmgr_quickpanel_handler_geometry_set(int x,
                                            int y,
                                            int width,
                                            int height)
{
   if (!_pol_quickpanel) return EINA_FALSE;

   _pol_quickpanel->handler_geo.x = x;
   _pol_quickpanel->handler_geo.y = y;
   _pol_quickpanel->handler_geo.w = width;
   _pol_quickpanel->handler_geo.h = height;

   return EINA_TRUE;
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
e_mod_qpmgr_quickpanel_init(void)
{
   E_LIST_HANDLER_APPEND(quickpanel_handlers, E_EVENT_COMP_OBJECT_ADD,
                         _quickpanel_cb_comp_object_add, NULL);
   E_CLIENT_HOOK_APPEND(quickpanel_hooks, E_CLIENT_HOOK_DEL,
                        _quickpanel_hook_client_del, NULL);
}

EINTERN void
e_mod_qpmgr_quickpanel_shutdown(void)
{
   E_FREE_LIST(quickpanel_hooks, e_client_hook_del);
   E_FREE_LIST(quickpanel_handlers, ecore_event_handler_del);
}
