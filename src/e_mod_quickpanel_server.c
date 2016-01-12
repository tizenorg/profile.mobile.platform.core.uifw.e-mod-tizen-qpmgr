#define E_COMP_WL
#include "e.h"
#include <wayland-server.h>
#include <Ecore_Wayland.h>
#include "quickpanel-server-protocol.h"
#include "e_mod_main.h"

//Quickpanel Policy Data
typedef struct _E_Quickpanel E_Quickpanel;

struct _E_Quickpanel
{
   E_Client *ec;

   struct
   {
      int x;
      int y;
      int width;
      int height;
   } handler_geo; // handler geometry
};

static E_Quickpanel *_e_quickpanel = NULL;

//internal functions
void _e_quickpanel_set_surface(struct wl_client *client, struct wl_resource *resource, struct wl_resource *surface);
void _e_quickpanel_set_handler_geometry(struct wl_client *client, struct wl_resource *resource, uint32_t x, uint32_t y, uint32_t width, uint32_t height);

static const struct quickpanel_interface _e_quickpanel_interface =
{
   _e_quickpanel_set_surface,
   _e_quickpanel_set_handler_geometry
};

void _e_quickpanel_set_surface(struct wl_client *client,
                               struct wl_resource *resource,
                               struct wl_resource *surface_resource)
{
   E_Pixmap *ep;
   E_Client *ec;
   E_Comp_Client_Data *cdata;

   /* get the pixmap from this surface so we can find the client */
   if (!(ep = wl_resource_get_user_data(surface_resource)))
     {
        wl_resource_post_error(surface_resource,
                               WL_DISPLAY_ERROR_INVALID_OBJECT,
                               "No Pixmap Set On Surface");
        return;
     }

   /* make sure it's a wayland pixmap */
   if (e_pixmap_type_get(ep) != E_PIXMAP_TYPE_WL) return;

   /* find the client for this pixmap */
   if (!(ec = e_pixmap_client_get(ep)))
     ec = e_pixmap_find_client(E_PIXMAP_TYPE_WL, e_pixmap_window_get(ep));

   if (!_e_quickpanel) return;

   _e_quickpanel->ec = ec;
}

void _e_quickpanel_set_handler_geometry(struct wl_client *client,
                                        struct wl_resource *resource,
                                        uint32_t x,
                                        uint32_t y,
                                        uint32_t width,
                                        uint32_t height)
{
   struct wl_client *wc;

   if (!_e_quickpanel) return;
   if (!_e_quickpanel->ec) return;

   E_OBJECT_CHECK(_e_quickpanel->ec);

   if (e_object_is_del(E_OBJECT(_e_quickpanel->ec))) return;

   wc = wl_resource_get_client(_e_quickpanel->ec->comp_data->surface);
   if (client != wc) return;

   _e_quickpanel->handler_geo.x = x;
   _e_quickpanel->handler_geo.y = y;
   _e_quickpanel->handler_geo.width = width;
   _e_quickpanel->handler_geo.height = height;

   e_mod_qpmgr_quickpanel_handler_geometry_set(x, y, width, height);
}

static void
_e_quickpanel_cb_bind(struct wl_client *client, void *data, uint32_t version, uint32_t id)
{
   struct wl_resource *res;

   if (!(res = wl_resource_create(client, &quickpanel_interface, MIN(version, 1), id)))
     {
        ERR("Could not create quickpoanel resource: %m");
        wl_client_post_no_memory(client);
        return;
     }

   wl_resource_set_implementation(res, &_e_quickpanel_interface, NULL, NULL);
}


EINTERN void
e_mod_qpmgr_quickpanel_server_init(void)
{
   E_Quickpanel *qp;

   if (!e_comp_wl) return;
   if (!e_comp_wl->wl.disp) return;

   if (!wl_global_create(e_comp_wl->wl.disp, &quickpanel_interface, 1, NULL, _e_quickpanel_cb_bind))
     {
        ERR("Could not add quickpanel to wayland globals: %m");
        return;
     }

   qp =  E_NEW(E_Quickpanel, 1);
   if (!qp) return;

   _e_quickpanel = qp;
}

EINTERN void
e_mod_qpmgr_quickpanel_server_shutdown(void)
{
   if (_e_quickpanel)
     {
        E_FREE(_e_quickpanel);
        _e_quickpanel = NULL;
     }
   return;
}
