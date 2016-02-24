/**
 * @file
 *
 * @brief
 *
 * @copyright BSD License (see doc/COPYING or http://www.libelektra.org)
 */

#include <kdberrors.h>
#include <kdbplugin.h>

#include <iconv.h>
#include <langinfo.h>
#include <locale.h>

#include <stdlib.h>
#include <string.h>

#define UTF8_TO 1
#define UTF8_FROM 0


#define BACKENDNAME "iconv"
#define BACKENDVERSION "0.0.1"

int kdbbNeedsUTF8Conversion (Plugin * handle);
int kdbbUTF8Engine (Plugin * handle, int direction, char ** string, size_t * inputOutputByteSize);

int elektraIconvGet (Plugin * handle, KeySet * ks, Key * parentKey);
int elektraIconvSet (Plugin * handle, KeySet * ks, Key * parentKey);
Plugin * ELEKTRA_PLUGIN_EXPORT (iconv);
