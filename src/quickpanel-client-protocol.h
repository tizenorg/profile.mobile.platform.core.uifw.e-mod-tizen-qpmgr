#ifndef QUICKPANEL_CLIENT_PROTOCOL_H
#define QUICKPANEL_CLIENT_PROTOCOL_H

#ifdef  __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stddef.h>
#include "wayland-client.h"

struct wl_client;
struct wl_resource;

struct quickpanel;

extern const struct wl_interface quickpanel_interface;

#define QUICKPANEL_SET_SURFACE	0
#define QUICKPANEL_SET_HANDLER_GEOMETRY	1

static inline void
quickpanel_set_user_data(struct quickpanel *quickpanel, void *user_data)
{
	wl_proxy_set_user_data((struct wl_proxy *) quickpanel, user_data);
}

static inline void *
quickpanel_get_user_data(struct quickpanel *quickpanel)
{
	return wl_proxy_get_user_data((struct wl_proxy *) quickpanel);
}

static inline void
quickpanel_destroy(struct quickpanel *quickpanel)
{
	wl_proxy_destroy((struct wl_proxy *) quickpanel);
}

static inline void
quickpanel_set_surface(struct quickpanel *quickpanel, struct wl_surface *surface)
{
	wl_proxy_marshal((struct wl_proxy *) quickpanel,
			 QUICKPANEL_SET_SURFACE, surface);
}

static inline void
quickpanel_set_handler_geometry(struct quickpanel *quickpanel, uint32_t x, uint32_t y, uint32_t width, uint32_t height)
{
	wl_proxy_marshal((struct wl_proxy *) quickpanel,
			 QUICKPANEL_SET_HANDLER_GEOMETRY, x, y, width, height);
}

#ifdef  __cplusplus
}
#endif

#endif
