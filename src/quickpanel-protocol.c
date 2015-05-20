#include <stdlib.h>
#include <stdint.h>
#include "wayland-util.h"

extern const struct wl_interface wl_surface_interface;

static const struct wl_interface *types[] = {
	NULL,
	NULL,
	NULL,
	NULL,
	&wl_surface_interface,
};

static const struct wl_message quickpanel_requests[] = {
	{ "set_surface", "o", types + 4 },
	{ "set_handler_geometry", "uuuu", types + 0 },
};

WL_EXPORT const struct wl_interface quickpanel_interface = {
	"quickpanel", 1,
	2, quickpanel_requests,
	0, NULL,
};

