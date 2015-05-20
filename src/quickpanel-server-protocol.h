#ifndef QUICKPANEL_SERVER_PROTOCOL_H
#define QUICKPANEL_SERVER_PROTOCOL_H

#ifdef  __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stddef.h>
#include "wayland-server.h"

struct wl_client;
struct wl_resource;

struct quickpanel;

extern const struct wl_interface quickpanel_interface;

struct quickpanel_interface {
	/**
	 * set_surface - (none)
	 * @surface: (none)
	 */
	void (*set_surface)(struct wl_client *client,
			    struct wl_resource *resource,
			    struct wl_resource *surface);
	/**
	 * set_handler_geometry - (none)
	 * @x: (none)
	 * @y: (none)
	 * @width: (none)
	 * @height: (none)
	 */
	void (*set_handler_geometry)(struct wl_client *client,
				     struct wl_resource *resource,
				     uint32_t x,
				     uint32_t y,
				     uint32_t width,
				     uint32_t height);
};


#ifdef  __cplusplus
}
#endif

#endif
