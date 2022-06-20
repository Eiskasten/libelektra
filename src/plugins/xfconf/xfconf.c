/**
 * @file
 *
 * @brief Source for xfconf plugin
 *
 * @copyright BSD License (see LICENSE.md or https://www.libelektra.org)
 *
 */

#include "xfconf.h"

#include <kdbhelper.h>
#include <kdblogger.h>
#include <libgen.h>
#include <xfconf/xfconf.h>

int elektraXfconfOpen (Plugin * handle ELEKTRA_UNUSED, Key * errorKey ELEKTRA_UNUSED)
{
	ELEKTRA_LOG ("try to initialize xfconf\n");
	GError * err = NULL;
	if (xfconf_init (&err))
	{
		ELEKTRA_LOG_DEBUG ("succeed initielize xfconf\n");
		return ELEKTRA_PLUGIN_STATUS_SUCCESS;
	}
	else
	{
		ELEKTRA_LOG ("unable to initialize xfconf(%d): %s\n", err->code, err->message);
		g_error_free (err);
		return ELEKTRA_PLUGIN_STATUS_ERROR;
	}
}

int elektraXfconfClose (Plugin * handle ELEKTRA_UNUSED, Key * errorKey ELEKTRA_UNUSED)
{
	xfconf_shutdown ();
	return ELEKTRA_PLUGIN_STATUS_SUCCESS;
}

int elektraXfconfGet (Plugin * handle ELEKTRA_UNUSED, KeySet * returned, Key * parentKey)
{
	ELEKTRA_LOG ("issued get\n");
	if (!elektraStrCmp (keyName (parentKey), "system:/elektra/modules/xfconf"))
	{
		ELEKTRA_LOG_DEBUG ("getting system modules values\n");
		KeySet * contract =
			ksNew (30, keyNew ("system:/elektra/modules/xfconf", KEY_VALUE, "xfconf plugin waits for your orders", KEY_END),
			       keyNew ("system:/elektra/modules/xfconf/exports", KEY_END),
			       keyNew ("system:/elektra/modules/xfconf/exports/open", KEY_FUNC, elektraXfconfOpen, KEY_END),
			       keyNew ("system:/elektra/modules/xfconf/exports/close", KEY_FUNC, elektraXfconfClose, KEY_END),
			       keyNew ("system:/elektra/modules/xfconf/exports/get", KEY_FUNC, elektraXfconfGet, KEY_END),
			       keyNew ("system:/elektra/modules/xfconf/exports/set", KEY_FUNC, elektraXfconfSet, KEY_END),
			       keyNew ("system:/elektra/modules/xfconf/exports/commit", KEY_FUNC, elektraXfconfCommit, KEY_END),
			       keyNew ("system:/elektra/modules/xfconf/exports/error", KEY_FUNC, elektraXfconfError, KEY_END),
			       keyNew ("system:/elektra/modules/xfconf/exports/checkconf", KEY_FUNC, elektraXfconfCheckConf, KEY_END),
#include ELEKTRA_README
			       keyNew ("system:/elektra/modules/xfconf/infos/version", KEY_VALUE, PLUGINVERSION, KEY_END), KS_END);
		ksAppend (returned, contract);
		ksDel (contract);

		return ELEKTRA_PLUGIN_STATUS_SUCCESS;
	}
	// get all keys

	//	KeySet * config = elektraPluginGetConfig (handle);
	// todo: remove workaround which requires a channel to exist as a file
	char * absolutePath = elektraStrDup (keyString (parentKey));
	const char * channelName = basename (absolutePath);
	ELEKTRA_LOG_DEBUG ("fetch keys from channel: %s\n", channelName);
	XfconfChannel * channel = xfconf_channel_get (channelName);
	if (channel == NULL)
	{
		ELEKTRA_LOG_DEBUG ("retrieved NULL attempting getting channel: %s\n", channelName);
	}
	GHashTable * properties = xfconf_channel_get_properties (channel, NULL);
	if (properties == NULL)
	{
		ELEKTRA_LOG_DEBUG ("retrieved NULL attempting getting properties\n");
	}
	GList * channelKeys = g_hash_table_get_keys (properties);
	while (channelKeys != NULL)
	{
		char * keyName = elektraStrDup (channelKeys->data);
		Key * key = keyNew (keyName, KEY_END);
		char * keyValue = g_hash_table_lookup (properties, channelKeys->data);
		ELEKTRA_LOG_DEBUG ("found %s -> %s\n", keyName, keyValue);
		keySetString (key, keyValue);
		ksAppendKey (returned, key);
		channelKeys = channelKeys->next;
	}

	return ELEKTRA_PLUGIN_STATUS_NO_UPDATE;
}

int elektraXfconfSet (Plugin * handle ELEKTRA_UNUSED, KeySet * returned ELEKTRA_UNUSED, Key * parentKey ELEKTRA_UNUSED)
{
	// set all keys
	// this function is optional

	return ELEKTRA_PLUGIN_STATUS_NO_UPDATE;
}

int elektraXfconfError (Plugin * handle ELEKTRA_UNUSED, KeySet * returned ELEKTRA_UNUSED, Key * parentKey ELEKTRA_UNUSED)
{
	// handle errors (commit failed)
	// this function is optional

	return ELEKTRA_PLUGIN_STATUS_SUCCESS;
}

int elektraXfconfCommit (Plugin * handle ELEKTRA_UNUSED, KeySet * returned ELEKTRA_UNUSED, Key * parentKey ELEKTRA_UNUSED)
{
	// commit changes
	// this function is optional

	return ELEKTRA_PLUGIN_STATUS_SUCCESS;
}

int elektraXfconfCheckConf (Key * errorKey ELEKTRA_UNUSED, KeySet * conf ELEKTRA_UNUSED)
{
	// validate plugin configuration
	// this function is optional

	return ELEKTRA_PLUGIN_STATUS_NO_UPDATE;
}

Plugin * ELEKTRA_PLUGIN_EXPORT
{
	// clang-format off
	return elektraPluginExport ("xfconf",
		ELEKTRA_PLUGIN_OPEN,	&elektraXfconfOpen,
		ELEKTRA_PLUGIN_CLOSE,	&elektraXfconfClose,
		ELEKTRA_PLUGIN_GET,	&elektraXfconfGet,
		ELEKTRA_PLUGIN_SET,	&elektraXfconfSet,
		ELEKTRA_PLUGIN_COMMIT,  &elektraXfconfCommit,
		ELEKTRA_PLUGIN_ERROR,	&elektraXfconfError,
		ELEKTRA_PLUGIN_END);
}
