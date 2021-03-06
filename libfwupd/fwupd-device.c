/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*-
 *
 * Copyright (C) 2015-2017 Richard Hughes <richard@hughsie.com>
 *
 * Licensed under the GNU Lesser General Public License Version 2.1
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301 USA
 */

#include "config.h"

#include <glib-object.h>
#include <gio/gio.h>
#include <string.h>

#include "fwupd-common-private.h"
#include "fwupd-enums-private.h"
#include "fwupd-error.h"
#include "fwupd-device-private.h"

static void fwupd_device_finalize	 (GObject *object);

typedef struct {
	gchar				*id;
	guint64				 created;
	guint64				 modified;
	guint64				 flags;
	gchar				*appstream_id;
	GPtrArray			*guids;
	gchar				*name;
	gchar				*summary;
	gchar				*description;
	gchar				*vendor;
	gchar				*homepage;
	gchar				*provider;
	gchar				*version;
	gchar				*version_lowest;
	gchar				*version_bootloader;
	GPtrArray			*checksums;
	guint32				 flashes_left;
} FwupdDevicePrivate;

G_DEFINE_TYPE_WITH_PRIVATE (FwupdDevice, fwupd_device, G_TYPE_OBJECT)
#define GET_PRIVATE(o) (fwupd_device_get_instance_private (o))

/**
 * fwupd_device_get_checksums:
 * @device: A #FwupdDevice
 *
 * Gets the device checksums.
 *
 * Returns: (element-type utf8) (transfer none): the checksums, which may be empty
 *
 * Since: 0.9.3
 **/
GPtrArray *
fwupd_device_get_checksums (FwupdDevice *device)
{
	FwupdDevicePrivate *priv = GET_PRIVATE (device);
	g_return_val_if_fail (FWUPD_IS_DEVICE (device), NULL);
	return priv->checksums;
}

/**
 * fwupd_device_add_checksum:
 * @device: A #FwupdDevice
 * @checksum: the device checksum
 *
 * Sets the device checksum.
 *
 * Since: 0.9.3
 **/
void
fwupd_device_add_checksum (FwupdDevice *device, const gchar *checksum)
{
	FwupdDevicePrivate *priv = GET_PRIVATE (device);
	g_return_if_fail (FWUPD_IS_DEVICE (device));
	g_return_if_fail (checksum != NULL);
	for (guint i = 0; i < priv->checksums->len; i++) {
		const gchar *checksum_tmp = g_ptr_array_index (priv->checksums, i);
		if (g_strcmp0 (checksum_tmp, checksum) == 0)
			return;
	}
	g_ptr_array_add (priv->checksums, g_strdup (checksum));
}

/**
 * fwupd_device_get_summary:
 * @device: A #FwupdDevice
 *
 * Gets the device summary.
 *
 * Returns: the device summary, or %NULL if unset
 *
 * Since: 0.9.3
 **/
const gchar *
fwupd_device_get_summary (FwupdDevice *device)
{
	FwupdDevicePrivate *priv = GET_PRIVATE (device);
	g_return_val_if_fail (FWUPD_IS_DEVICE (device), NULL);
	return priv->summary;
}

/**
 * fwupd_device_set_summary:
 * @device: A #FwupdDevice
 * @summary: the device one line summary
 *
 * Sets the device summary.
 *
 * Since: 0.9.3
 **/
void
fwupd_device_set_summary (FwupdDevice *device, const gchar *summary)
{
	FwupdDevicePrivate *priv = GET_PRIVATE (device);
	g_return_if_fail (FWUPD_IS_DEVICE (device));
	g_free (priv->summary);
	priv->summary = g_strdup (summary);
}

/**
 * fwupd_device_get_id:
 * @device: A #FwupdDevice
 *
 * Gets the ID.
 *
 * Returns: the ID, or %NULL if unset
 *
 * Since: 0.9.3
 **/
const gchar *
fwupd_device_get_id (FwupdDevice *device)
{
	FwupdDevicePrivate *priv = GET_PRIVATE (device);
	g_return_val_if_fail (FWUPD_IS_DEVICE (device), NULL);
	return priv->id;
}

/**
 * fwupd_device_set_id:
 * @device: A #FwupdDevice
 * @device_id: the device ID, e.g. "USB:foo"
 *
 * Sets the ID.
 *
 * Since: 0.9.3
 **/
void
fwupd_device_set_id (FwupdDevice *device, const gchar *id)
{
	FwupdDevicePrivate *priv = GET_PRIVATE (device);
	g_return_if_fail (FWUPD_IS_DEVICE (device));
	g_free (priv->id);
	priv->id = g_strdup (id);
}

/**
 * fwupd_device_get_guids:
 * @device: A #FwupdDevice
 *
 * Gets the GUIDs.
 *
 * Returns: (element-type utf8) (transfer none): the GUIDs
 *
 * Since: 0.9.3
 **/
GPtrArray *
fwupd_device_get_guids (FwupdDevice *device)
{
	FwupdDevicePrivate *priv = GET_PRIVATE (device);
	g_return_val_if_fail (FWUPD_IS_DEVICE (device), NULL);
	return priv->guids;
}

/**
 * fwupd_device_has_guid:
 * @device: A #FwupdDevice
 * @guid: the GUID, e.g. "2082b5e0-7a64-478a-b1b2-e3404fab6dad"
 *
 * Finds out if the device has this specific GUID.
 *
 * Returns: %TRUE if the GUID is found
 *
 * Since: 0.9.3
 **/
gboolean
fwupd_device_has_guid (FwupdDevice *device, const gchar *guid)
{
	FwupdDevicePrivate *priv = GET_PRIVATE (device);
	guint i;

	g_return_val_if_fail (FWUPD_IS_DEVICE (device), FALSE);

	for (i = 0; i < priv->guids->len; i++) {
		const gchar *guid_tmp = g_ptr_array_index (priv->guids, i);
		if (g_strcmp0 (guid, guid_tmp) == 0)
			return TRUE;
	}
	return FALSE;
}

/**
 * fwupd_device_add_guid:
 * @device: A #FwupdDevice
 * @guid: the GUID, e.g. "2082b5e0-7a64-478a-b1b2-e3404fab6dad"
 *
 * Adds the GUID if it does not already exist.
 *
 * Since: 0.9.3
 **/
void
fwupd_device_add_guid (FwupdDevice *device, const gchar *guid)
{
	FwupdDevicePrivate *priv = GET_PRIVATE (device);
	g_return_if_fail (FWUPD_IS_DEVICE (device));
	if (fwupd_device_has_guid (device, guid))
		return;
	g_ptr_array_add (priv->guids, g_strdup (guid));
}

/**
 * fwupd_device_get_guid_default:
 * @device: A #FwupdDevice
 *
 * Gets the default GUID.
 *
 * Returns: the GUID, or %NULL if unset
 *
 * Since: 0.9.3
 **/
const gchar *
fwupd_device_get_guid_default (FwupdDevice *device)
{
	FwupdDevicePrivate *priv = GET_PRIVATE (device);
	g_return_val_if_fail (FWUPD_IS_DEVICE (device), NULL);
	if (priv->guids->len == 0)
		return NULL;
	return g_ptr_array_index (priv->guids, 0);
}

/**
 * fwupd_device_get_name:
 * @device: A #FwupdDevice
 *
 * Gets the device name.
 *
 * Returns: the device name, or %NULL if unset
 *
 * Since: 0.9.3
 **/
const gchar *
fwupd_device_get_name (FwupdDevice *device)
{
	FwupdDevicePrivate *priv = GET_PRIVATE (device);
	g_return_val_if_fail (FWUPD_IS_DEVICE (device), NULL);
	return priv->name;
}

/**
 * fwupd_device_set_name:
 * @device: A #FwupdDevice
 * @device_name: the device name, e.g. "ColorHug2"
 *
 * Sets the device name.
 *
 * Since: 0.9.3
 **/
void
fwupd_device_set_name (FwupdDevice *device, const gchar *name)
{
	FwupdDevicePrivate *priv = GET_PRIVATE (device);
	g_return_if_fail (FWUPD_IS_DEVICE (device));
	g_free (priv->name);
	priv->name = g_strdup (name);
}

/**
 * fwupd_device_get_vendor:
 * @device: A #FwupdDevice
 *
 * Gets the device vendor.
 *
 * Returns: the device vendor, or %NULL if unset
 *
 * Since: 0.9.3
 **/
const gchar *
fwupd_device_get_vendor (FwupdDevice *device)
{
	FwupdDevicePrivate *priv = GET_PRIVATE (device);
	g_return_val_if_fail (FWUPD_IS_DEVICE (device), NULL);
	return priv->vendor;
}

/**
 * fwupd_device_set_vendor:
 * @device: A #FwupdDevice
 * @device_vendor: the description
 *
 * Sets the device vendor.
 *
 * Since: 0.9.3
 **/
void
fwupd_device_set_vendor (FwupdDevice *device, const gchar *vendor)
{
	FwupdDevicePrivate *priv = GET_PRIVATE (device);
	g_return_if_fail (FWUPD_IS_DEVICE (device));
	g_free (priv->vendor);
	priv->vendor = g_strdup (vendor);
}

/**
 * fwupd_device_get_description:
 * @device: A #FwupdDevice
 *
 * Gets the device description in AppStream markup format.
 *
 * Returns: the device description, or %NULL if unset
 *
 * Since: 0.9.3
 **/
const gchar *
fwupd_device_get_description (FwupdDevice *device)
{
	FwupdDevicePrivate *priv = GET_PRIVATE (device);
	g_return_val_if_fail (FWUPD_IS_DEVICE (device), NULL);
	return priv->description;
}

/**
 * fwupd_device_set_description:
 * @device: A #FwupdDevice
 * @device_description: the description in AppStream markup format
 *
 * Sets the device description.
 *
 * Since: 0.9.3
 **/
void
fwupd_device_set_description (FwupdDevice *device, const gchar *description)
{
	FwupdDevicePrivate *priv = GET_PRIVATE (device);
	g_return_if_fail (FWUPD_IS_DEVICE (device));
	g_free (priv->description);
	priv->description = g_strdup (description);
}

/**
 * fwupd_device_get_version:
 * @device: A #FwupdDevice
 *
 * Gets the device version.
 *
 * Returns: the device version, or %NULL if unset
 *
 * Since: 0.9.3
 **/
const gchar *
fwupd_device_get_version (FwupdDevice *device)
{
	FwupdDevicePrivate *priv = GET_PRIVATE (device);
	g_return_val_if_fail (FWUPD_IS_DEVICE (device), NULL);
	return priv->version;
}

/**
 * fwupd_device_set_version:
 * @device: A #FwupdDevice
 * @device_version: the device version, e.g. "1.2.3"
 *
 * Sets the device version.
 *
 * Since: 0.9.3
 **/
void
fwupd_device_set_version (FwupdDevice *device, const gchar *version)
{
	FwupdDevicePrivate *priv = GET_PRIVATE (device);
	g_return_if_fail (FWUPD_IS_DEVICE (device));
	g_free (priv->version);
	priv->version = g_strdup (version);
}

/**
 * fwupd_device_get_version_lowest:
 * @device: A #FwupdDevice
 *
 * Gets the lowest version of firmware the device will accept.
 *
 * Returns: the device version_lowest, or %NULL if unset
 *
 * Since: 0.9.3
 **/
const gchar *
fwupd_device_get_version_lowest (FwupdDevice *device)
{
	FwupdDevicePrivate *priv = GET_PRIVATE (device);
	g_return_val_if_fail (FWUPD_IS_DEVICE (device), NULL);
	return priv->version_lowest;
}

/**
 * fwupd_device_set_version_lowest:
 * @device: A #FwupdDevice
 * @device_version_lowest: the description
 *
 * Sets the lowest version of firmware the device will accept.
 *
 * Since: 0.9.3
 **/
void
fwupd_device_set_version_lowest (FwupdDevice *device, const gchar *version_lowest)
{
	FwupdDevicePrivate *priv = GET_PRIVATE (device);
	g_return_if_fail (FWUPD_IS_DEVICE (device));
	g_free (priv->version_lowest);
	priv->version_lowest = g_strdup (version_lowest);
}

/**
 * fwupd_device_get_version_bootloader:
 * @device: A #FwupdDevice
 *
 * Gets the version of the bootloader.
 *
 * Returns: the device version_bootloader, or %NULL if unset
 *
 * Since: 0.9.3
 **/
const gchar *
fwupd_device_get_version_bootloader (FwupdDevice *device)
{
	FwupdDevicePrivate *priv = GET_PRIVATE (device);
	g_return_val_if_fail (FWUPD_IS_DEVICE (device), NULL);
	return priv->version_bootloader;
}

/**
 * fwupd_device_set_version_bootloader:
 * @device: A #FwupdDevice
 * @device_version_bootloader: the description
 *
 * Sets the bootloader version.
 *
 * Since: 0.9.3
 **/
void
fwupd_device_set_version_bootloader (FwupdDevice *device, const gchar *version_bootloader)
{
	FwupdDevicePrivate *priv = GET_PRIVATE (device);
	g_return_if_fail (FWUPD_IS_DEVICE (device));
	g_free (priv->version_bootloader);
	priv->version_bootloader = g_strdup (version_bootloader);
}

/**
 * fwupd_device_device_get_flashes_left:
 * @device: A #FwupdDevice
 *
 * Gets the number of flash cycles left on the device
 *
 * Returns: the flash cycles left, or %NULL if unset
 *
 * Since: 0.9.3
 **/
guint32
fwupd_device_get_flashes_left (FwupdDevice *device)
{
	FwupdDevicePrivate *priv = GET_PRIVATE (device);
	g_return_val_if_fail (FWUPD_IS_DEVICE (device), 0);
	return priv->flashes_left;
}

/**
 * fwupd_device_device_set_flashes_left:
 * @device: A #FwupdDevice
 * @flashes_left: the description
 *
 * Sets the number of flash cycles left on the device
 *
 * Since: 0.9.3
 **/
void
fwupd_device_set_flashes_left (FwupdDevice *device, guint32 flashes_left)
{
	FwupdDevicePrivate *priv = GET_PRIVATE (device);
	g_return_if_fail (FWUPD_IS_DEVICE (device));
	priv->flashes_left = flashes_left;
}

/**
 * fwupd_device_get_provider:
 * @device: A #FwupdDevice
 *
 * Gets the device provider.
 *
 * Returns: the device provider, or %NULL if unset
 *
 * Since: 0.9.3
 **/
const gchar *
fwupd_device_get_provider (FwupdDevice *device)
{
	FwupdDevicePrivate *priv = GET_PRIVATE (device);
	g_return_val_if_fail (FWUPD_IS_DEVICE (device), NULL);
	return priv->provider;
}

/**
 * fwupd_device_set_provider:
 * @device: A #FwupdDevice
 * @device_provider: the provider name, e.g. "colorhug"
 *
 * Sets the device provider.
 *
 * Since: 0.9.3
 **/
void
fwupd_device_set_provider (FwupdDevice *device, const gchar *provider)
{
	FwupdDevicePrivate *priv = GET_PRIVATE (device);
	g_return_if_fail (FWUPD_IS_DEVICE (device));
	g_free (priv->provider);
	priv->provider = g_strdup (provider);
}

/**
 * fwupd_device_get_flags:
 * @device: A #FwupdDevice
 *
 * Gets the device flags.
 *
 * Returns: the device flags, or 0 if unset
 *
 * Since: 0.9.3
 **/
guint64
fwupd_device_get_flags (FwupdDevice *device)
{
	FwupdDevicePrivate *priv = GET_PRIVATE (device);
	g_return_val_if_fail (FWUPD_IS_DEVICE (device), 0);
	return priv->flags;
}

/**
 * fwupd_device_set_flags:
 * @device: A #FwupdDevice
 * @device_flags: the device flags, e.g. %FWUPD_DEVICE_FLAG_REQUIRE_AC
 *
 * Sets the device flags.
 *
 * Since: 0.9.3
 **/
void
fwupd_device_set_flags (FwupdDevice *device, guint64 flags)
{
	FwupdDevicePrivate *priv = GET_PRIVATE (device);
	g_return_if_fail (FWUPD_IS_DEVICE (device));
	priv->flags = flags;
}

/**
 * fwupd_device_add_flag:
 * @device: A #FwupdDevice
 * @flag: the #FwupdDeviceFlags
 *
 * Adds a specific device flag to the device.
 *
 * Since: 0.9.3
 **/
void
fwupd_device_add_flag (FwupdDevice *device, FwupdDeviceFlags flag)
{
	FwupdDevicePrivate *priv = GET_PRIVATE (device);
	g_return_if_fail (FWUPD_IS_DEVICE (device));
	priv->flags |= flag;
}

/**
 * fwupd_device_remove_flag:
 * @device: A #FwupdDevice
 * @flag: the #FwupdDeviceFlags
 *
 * Removes a specific device flag from the device.
 *
 * Since: 0.9.3
 **/
void
fwupd_device_remove_flag (FwupdDevice *device, FwupdDeviceFlags flag)
{
	FwupdDevicePrivate *priv = GET_PRIVATE (device);
	g_return_if_fail (FWUPD_IS_DEVICE (device));
	priv->flags &= ~flag;
}

/**
 * fwupd_device_has_flag:
 * @device: A #FwupdDevice
 * @flag: the #FwupdDeviceFlags
 *
 * Finds if the device has a specific device flag.
 *
 * Returns: %TRUE if the flag is set
 *
 * Since: 0.9.3
 **/
gboolean
fwupd_device_has_flag (FwupdDevice *device, FwupdDeviceFlags flag)
{
	FwupdDevicePrivate *priv = GET_PRIVATE (device);
	g_return_val_if_fail (FWUPD_IS_DEVICE (device), FALSE);
	return (priv->flags & flag) > 0;
}

/**
 * fwupd_device_get_created:
 * @device: A #FwupdDevice
 *
 * Gets when the device was created.
 *
 * Returns: the UNIX time, or 0 if unset
 *
 * Since: 0.9.3
 **/
guint64
fwupd_device_get_created (FwupdDevice *device)
{
	FwupdDevicePrivate *priv = GET_PRIVATE (device);
	g_return_val_if_fail (FWUPD_IS_DEVICE (device), 0);
	return priv->created;
}


/**
 * fwupd_device_set_created:
 * @device: A #FwupdDevice
 * @device_created: the UNIX time
 *
 * Sets when the device was created.
 *
 * Since: 0.9.3
 **/
void
fwupd_device_set_created (FwupdDevice *device, guint64 created)
{
	FwupdDevicePrivate *priv = GET_PRIVATE (device);
	g_return_if_fail (FWUPD_IS_DEVICE (device));
	priv->created = created;
}

/**
 * fwupd_device_get_modified:
 * @device: A #FwupdDevice
 *
 * Gets when the device was modified.
 *
 * Returns: the UNIX time, or 0 if unset
 *
 * Since: 0.9.3
 **/
guint64
fwupd_device_get_modified (FwupdDevice *device)
{
	FwupdDevicePrivate *priv = GET_PRIVATE (device);
	g_return_val_if_fail (FWUPD_IS_DEVICE (device), 0);
	return priv->modified;
}

/**
 * fwupd_device_set_modified:
 * @device: A #FwupdDevice
 * @device_modified: the UNIX time
 *
 * Sets when the device was modified.
 *
 * Since: 0.9.3
 **/
void
fwupd_device_set_modified (FwupdDevice *device, guint64 modified)
{
	FwupdDevicePrivate *priv = GET_PRIVATE (device);
	g_return_if_fail (FWUPD_IS_DEVICE (device));
	priv->modified = modified;
}

void
fwupd_device_to_variant_builder (FwupdDevice *device, GVariantBuilder *builder)
{
	FwupdDevicePrivate *priv = GET_PRIVATE (device);
	if (priv->guids->len > 0) {
		guint i;
		g_autoptr(GString) str = g_string_new ("");
		for (i = 0; i < priv->guids->len; i++) {
			const gchar *guid = g_ptr_array_index (priv->guids, i);
			g_string_append_printf (str, "%s,", guid);
		}
		if (str->len > 0)
			g_string_truncate (str, str->len - 1);
		g_variant_builder_add (builder, "{sv}",
				       FWUPD_RESULT_KEY_GUID,
				       g_variant_new_string (str->str));
	}
	if (priv->name != NULL) {
		g_variant_builder_add (builder, "{sv}",
				       FWUPD_RESULT_KEY_DEVICE_NAME,
				       g_variant_new_string (priv->name));
	}
	if (priv->vendor != NULL) {
		g_variant_builder_add (builder, "{sv}",
				       FWUPD_RESULT_KEY_DEVICE_VENDOR,
				       g_variant_new_string (priv->vendor));
	}
	if (priv->flags > 0) {
		g_variant_builder_add (builder, "{sv}",
				       FWUPD_RESULT_KEY_DEVICE_FLAGS,
				       g_variant_new_uint64 (priv->flags));
	}
	if (priv->created > 0) {
		g_variant_builder_add (builder, "{sv}",
				       FWUPD_RESULT_KEY_DEVICE_CREATED,
				       g_variant_new_uint64 (priv->created));
	}
	if (priv->modified > 0) {
		g_variant_builder_add (builder, "{sv}",
				       FWUPD_RESULT_KEY_DEVICE_MODIFIED,
				       g_variant_new_uint64 (priv->modified));
	}

	if (priv->description != NULL) {
		g_variant_builder_add (builder, "{sv}",
				       FWUPD_RESULT_KEY_DEVICE_DESCRIPTION,
				       g_variant_new_string (priv->description));
	}
	if (priv->checksums->len > 0) {
		g_autoptr(GString) str = g_string_new ("");
		for (guint i = 0; i < priv->checksums->len; i++) {
			const gchar *checksum = g_ptr_array_index (priv->checksums, i);
			g_string_append_printf (str, "%s,", checksum);
		}
		if (str->len > 0)
			g_string_truncate (str, str->len - 1);
		g_variant_builder_add (builder, "{sv}",
				       FWUPD_RESULT_KEY_DEVICE_CHECKSUM,
				       g_variant_new_string (str->str));
	}
	if (priv->provider != NULL) {
		g_variant_builder_add (builder, "{sv}",
				       FWUPD_RESULT_KEY_DEVICE_PLUGIN,
				       g_variant_new_string (priv->provider));
	}
	if (priv->version != NULL) {
		g_variant_builder_add (builder, "{sv}",
				       FWUPD_RESULT_KEY_DEVICE_VERSION,
				       g_variant_new_string (priv->version));
	}
	if (priv->version_lowest != NULL) {
		g_variant_builder_add (builder, "{sv}",
				       FWUPD_RESULT_KEY_DEVICE_VERSION_LOWEST,
				       g_variant_new_string (priv->version_lowest));
	}
	if (priv->version_bootloader != NULL) {
		g_variant_builder_add (builder, "{sv}",
				       FWUPD_RESULT_KEY_DEVICE_VERSION_BOOTLOADER,
				       g_variant_new_string (priv->version_bootloader));
	}
	if (priv->flashes_left > 0) {
		g_variant_builder_add (builder, "{sv}",
				       FWUPD_RESULT_KEY_DEVICE_FLASHES_LEFT,
				       g_variant_new_uint32 (priv->flashes_left));
	}
}

/**
 * fwupd_device_to_data:
 * @device: A #FwupdDevice
 * @type_string: The Gvariant type string, e.g. "a{sv}" or "(a{sv})"
 *
 * Creates a GVariant from the device data.
 *
 * Returns: the GVariant, or %NULL for error
 *
 * Since: 0.9.3
 **/
GVariant *
fwupd_device_to_data (FwupdDevice *device, const gchar *type_string)
{
	GVariantBuilder builder;

	g_return_val_if_fail (FWUPD_IS_DEVICE (device), NULL);
	g_return_val_if_fail (type_string != NULL, NULL);

	/* create an array with all the metadata in */
	g_variant_builder_init (&builder, G_VARIANT_TYPE_ARRAY);
	fwupd_device_to_variant_builder (device, &builder);

	/* supported types */
	if (g_strcmp0 (type_string, "a{sv}") == 0)
		return g_variant_new ("a{sv}", &builder);
	if (g_strcmp0 (type_string, "(a{sv})") == 0)
		return g_variant_new ("(a{sv})", &builder);
	return NULL;
}

void
fwupd_device_from_key_value (FwupdDevice *device, const gchar *key, GVariant *value)
{
	if (g_strcmp0 (key, FWUPD_RESULT_KEY_DEVICE_FLAGS) == 0) {
		fwupd_device_set_flags (device, g_variant_get_uint64 (value));
		return;
	}
	if (g_strcmp0 (key, FWUPD_RESULT_KEY_DEVICE_CREATED) == 0) {
		fwupd_device_set_created (device, g_variant_get_uint64 (value));
		return;
	}
	if (g_strcmp0 (key, FWUPD_RESULT_KEY_DEVICE_MODIFIED) == 0) {
		fwupd_device_set_modified (device, g_variant_get_uint64 (value));
		return;
	}
	if (g_strcmp0 (key, FWUPD_RESULT_KEY_GUID) == 0) {
		guint i;
		const gchar *guids = g_variant_get_string (value, NULL);
		g_auto(GStrv) split = g_strsplit (guids, ",", -1);
		for (i = 0; split[i] != NULL; i++)
			fwupd_device_add_guid (device, split[i]);
		return;
	}
	if (g_strcmp0 (key, FWUPD_RESULT_KEY_DEVICE_NAME) == 0) {
		fwupd_device_set_name (device, g_variant_get_string (value, NULL));
		return;
	}
	if (g_strcmp0 (key, FWUPD_RESULT_KEY_DEVICE_VENDOR) == 0) {
		fwupd_device_set_vendor (device, g_variant_get_string (value, NULL));
		return;
	}
	if (g_strcmp0 (key, FWUPD_RESULT_KEY_DEVICE_DESCRIPTION) == 0) {
		fwupd_device_set_description (device, g_variant_get_string (value, NULL));
		return;
	}
	if (g_strcmp0 (key, FWUPD_RESULT_KEY_DEVICE_CHECKSUM) == 0) {
		guint i;
		const gchar *checksums = g_variant_get_string (value, NULL);
		g_auto(GStrv) split = g_strsplit (checksums, ",", -1);
		for (i = 0; split[i] != NULL; i++)
			fwupd_device_add_checksum (device, split[i]);
		return;
	}
	if (g_strcmp0 (key, FWUPD_RESULT_KEY_DEVICE_PLUGIN) == 0) {
		fwupd_device_set_provider (device, g_variant_get_string (value, NULL));
		return;
	}
	if (g_strcmp0 (key, FWUPD_RESULT_KEY_DEVICE_VERSION) == 0) {
		fwupd_device_set_version (device, g_variant_get_string (value, NULL));
		return;
	}
	if (g_strcmp0 (key, FWUPD_RESULT_KEY_DEVICE_VERSION_LOWEST) == 0) {
		fwupd_device_set_version_lowest (device, g_variant_get_string (value, NULL));
		return;
	}
	if (g_strcmp0 (key, FWUPD_RESULT_KEY_DEVICE_VERSION_BOOTLOADER) == 0) {
		fwupd_device_set_version_bootloader (device, g_variant_get_string (value, NULL));
		return;
	}
	if (g_strcmp0 (key, FWUPD_RESULT_KEY_DEVICE_FLASHES_LEFT) == 0) {
		fwupd_device_set_flashes_left (device, g_variant_get_uint32 (value));
		return;
	}
}

static void
fwupd_pad_kv_str (GString *str, const gchar *key, const gchar *value)
{
	/* ignore */
	if (key == NULL || value == NULL)
		return;
	g_string_append_printf (str, "  %s: ", key);
	for (gsize i = strlen (key); i < 20; i++)
		g_string_append (str, " ");
	g_string_append_printf (str, "%s\n", value);
}

static void
fwupd_pad_kv_unx (GString *str, const gchar *key, guint64 value)
{
	g_autoptr(GDateTime) date = NULL;
	g_autofree gchar *tmp = NULL;

	/* ignore */
	if (value == 0)
		return;

	date = g_date_time_new_from_unix_utc ((gint64) value);
	tmp = g_date_time_format (date, "%F");
	fwupd_pad_kv_str (str, key, tmp);
}

static void
fwupd_pad_kv_dfl (GString *str, const gchar *key, guint64 device_flags)
{
	guint i;
	g_autoptr(GString) tmp = NULL;

	tmp = g_string_new ("");
	for (i = 0; i < 64; i++) {
		if ((device_flags & ((guint64) 1 << i)) == 0)
			continue;
		g_string_append_printf (tmp, "%s|",
					fwupd_device_flag_to_string ((guint64) 1 << i));
	}
	if (tmp->len == 0) {
		g_string_append (tmp, fwupd_device_flag_to_string (0));
	} else {
		g_string_truncate (tmp, tmp->len - 1);
	}
	fwupd_pad_kv_str (str, key, tmp->str);
}

static void
fwupd_pad_kv_int (GString *str, const gchar *key, guint32 value)
{
	g_autofree gchar *tmp = NULL;

	/* ignore */
	if (value == 0)
		return;
	tmp = g_strdup_printf("%" G_GUINT32_FORMAT, value);
	fwupd_pad_kv_str (str, key, tmp);
}

/**
 * fwupd_device_to_string:
 * @device: A #FwupdDevice
 *
 * Builds a text representation of the object.
 *
 * Returns: text, or %NULL for invalid
 *
 * Since: 0.9.3
 **/
gchar *
fwupd_device_to_string (FwupdDevice *device)
{
	FwupdDevicePrivate *priv = GET_PRIVATE (device);
	GString *str;

	g_return_val_if_fail (FWUPD_IS_DEVICE (device), NULL);

	str = g_string_new ("");
	for (guint i = 0; i < priv->guids->len; i++) {
		const gchar *guid = g_ptr_array_index (priv->guids, i);
		fwupd_pad_kv_str (str, FWUPD_RESULT_KEY_GUID, guid);
	}
	fwupd_pad_kv_str (str, FWUPD_RESULT_KEY_DEVICE_ID, priv->id);
	fwupd_pad_kv_str (str, FWUPD_RESULT_KEY_DEVICE_DESCRIPTION, priv->description);
	fwupd_pad_kv_str (str, FWUPD_RESULT_KEY_DEVICE_PLUGIN, priv->provider);
	fwupd_pad_kv_dfl (str, FWUPD_RESULT_KEY_DEVICE_FLAGS, priv->flags);
	for (guint i = 0; i < priv->checksums->len; i++) {
		const gchar *checksum = g_ptr_array_index (priv->checksums, i);
		g_autofree gchar *checksum_display = fwupd_checksum_format_for_display (checksum);
		fwupd_pad_kv_str (str, FWUPD_RESULT_KEY_DEVICE_CHECKSUM, checksum_display);
	}
	fwupd_pad_kv_str (str, FWUPD_RESULT_KEY_DEVICE_VENDOR, priv->vendor);
	fwupd_pad_kv_str (str, FWUPD_RESULT_KEY_DEVICE_VERSION, priv->version);
	fwupd_pad_kv_str (str, FWUPD_RESULT_KEY_DEVICE_VERSION_LOWEST, priv->version_lowest);
	fwupd_pad_kv_str (str, FWUPD_RESULT_KEY_DEVICE_VERSION_BOOTLOADER, priv->version_bootloader);
	if (priv->flashes_left < 2)
		fwupd_pad_kv_int (str, FWUPD_RESULT_KEY_DEVICE_FLASHES_LEFT, priv->flashes_left);
	fwupd_pad_kv_unx (str, FWUPD_RESULT_KEY_DEVICE_CREATED, priv->created);
	fwupd_pad_kv_unx (str, FWUPD_RESULT_KEY_DEVICE_MODIFIED, priv->modified);

	return g_string_free (str, FALSE);
}

static void
fwupd_device_class_init (FwupdDeviceClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);
	object_class->finalize = fwupd_device_finalize;
}

static void
fwupd_device_init (FwupdDevice *device)
{
	FwupdDevicePrivate *priv = GET_PRIVATE (device);
	priv->guids = g_ptr_array_new_with_free_func (g_free);
	priv->checksums = g_ptr_array_new_with_free_func (g_free);
}

static void
fwupd_device_finalize (GObject *object)
{
	FwupdDevice *device = FWUPD_DEVICE (object);
	FwupdDevicePrivate *priv = GET_PRIVATE (device);

	g_free (priv->description);
	g_free (priv->id);
	g_free (priv->name);
	g_free (priv->vendor);
	g_free (priv->provider);
	g_free (priv->version);
	g_free (priv->version_lowest);
	g_free (priv->version_bootloader);
	g_ptr_array_unref (priv->guids);
	g_ptr_array_unref (priv->checksums);

	G_OBJECT_CLASS (fwupd_device_parent_class)->finalize (object);
}

static void
fwupd_device_set_from_variant_iter (FwupdDevice *device, GVariantIter *iter)
{
	GVariant *value;
	const gchar *key;
	while (g_variant_iter_next (iter, "{&sv}", &key, &value)) {
		fwupd_device_from_key_value (device, key, value);
		g_variant_unref (value);
	}
}

/**
 * fwupd_device_new_from_data:
 * @data: a #GVariant
 *
 * Creates a new device using packed data.
 *
 * Returns: a new #FwupdDevice, or %NULL if @data was invalid
 *
 * Since: 0.9.3
 **/
FwupdDevice *
fwupd_device_new_from_data (GVariant *data)
{
	FwupdDevice *dev = NULL;
	const gchar *type_string;
	g_autoptr(GVariantIter) iter = NULL;

	/* format from GetDetails */
	type_string = g_variant_get_type_string (data);
	if (g_strcmp0 (type_string, "(a{sv})") == 0) {
		dev = fwupd_device_new ();
		g_variant_get (data, "(a{sv})", &iter);
		fwupd_device_set_from_variant_iter (dev, iter);
	} else if (g_strcmp0 (type_string, "a{sv}") == 0) {
		dev = fwupd_device_new ();
		g_variant_get (data, "a{sv}", &iter);
		fwupd_device_set_from_variant_iter (dev, iter);
	} else if (g_strcmp0 (type_string, "{sa{sv}}") == 0) {
		const gchar *id;
		dev = fwupd_device_new ();
		g_variant_get (data, "{&sa{sv}}", &id, &iter);
		fwupd_device_set_id (dev, id);
		fwupd_device_set_from_variant_iter (dev, iter);
	} else {
		g_warning ("type %s not known", type_string);
	}
	return dev;
}

/**
 * fwupd_device_new:
 *
 * Creates a new device.
 *
 * Returns: a new #FwupdDevice
 *
 * Since: 0.9.3
 **/
FwupdDevice *
fwupd_device_new (void)
{
	FwupdDevice *device;
	device = g_object_new (FWUPD_TYPE_DEVICE, NULL);
	return FWUPD_DEVICE (device);
}
