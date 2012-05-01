#ifdef HAVE_KDBCONFIG_H
#include "kdbconfig.h"
#endif

#include <stdlib.h>
#include <string.h>

#include <kdberrors.h>
#include <kdbmodule.h>
#include <exported_symbols.h>


int elektraModulesInit (KeySet *modules, Key *error)
{
	ksAppendKey (modules, keyNew ("system/elektra/modules", KEY_END));

	return 0;
}

static kdblib_symbol* elektraStaticLoad(const char *module)
{
	kdblib_symbol	*current;
	current = kdb_exported_syms;
	while ( current->name != NULL ) {
		/* Skip symbols, we're searching for
		 * the module name */
		if ( current->function == NULL && strcmp(current->name, module) == 0 ) {
			/* Go to the first symbol for this file */
			current++;
			return current;
		}

		current++;
	}

	return NULL;
}

static kdblib_symbol* elektraStaticSym(kdblib_symbol* handle, const char *symbol)
{
	kdblib_symbol	*current;

	current = handle;
	/* For each symbol about this module */
	while ( current->function != NULL ) {
		if ( strcmp(current->name, symbol) == 0 )
			return current;

		current++;
	}
	
	return NULL;
}

elektraPluginFactory elektraModulesLoad (KeySet *modules, const char *name, Key *error)
{
	Key *moduleKey = keyNew ("system/elektra/modules", KEY_END);
	keyAddBaseName (moduleKey, name);
	Key *lookup = ksLookup(modules, moduleKey, 0);
	if (lookup)
	{
		kdblib_symbol *module = (kdblib_symbol*) keyValue(lookup);
		keyDel (moduleKey);
		return (elektraPluginFactory)module->function;
	}

	kdblib_symbol* handle = elektraStaticLoad(name);

	if (handle == NULL)
	{
		char *errorText = malloc(strlen(name) + 40);
		strcpy (errorText, "Did not find module ");
		strcat (errorText, name);
		ELEKTRA_SET_ERROR(70, error, errorText);
		free (errorText);
		keyDel (moduleKey);
		return 0;
	}

	kdblib_symbol* module = elektraStaticSym(handle, "elektraPluginSymbol");

	if (module == NULL)
	{
		ELEKTRA_SET_ERROR(71, error, "no such symbol elektraPluginSymbol");
		return 0;
	}

	keySetBinary (moduleKey, module, sizeof (kdblib_symbol));
	ksAppendKey (modules, moduleKey);

	return (elektraPluginFactory)module->function;
}

int elektraModulesClose (KeySet *modules, Key *error)
{
	ksClear(modules);
	return 0;
}
