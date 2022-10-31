#include "c11r_parser.hpp"
#include "../c11r_lang.hpp"
#include "core/io/config_file.h"
//
//  ResourceFormatLoaderC11R
//


RES ResourceFormatLoaderC11R::load(const String &path, const String &original_path, Error *r_error)
{
    Ref<C11RScript> script;
    script.instance();
    if (r_error)
    {
        *r_error = OK;
    }
    print_line("Loading C11R Script with custom format loader");

    Ref<ConfigFile> file;
    file.instance();
    // load as a config file
    Error err = file->load(path);
    if(err != OK && r_error != NULL)
    {
        *r_error = err;
        return script;
    }
    // load into script
    script->load(file);
    return script;
}

void ResourceFormatLoaderC11R::get_recognized_extensions(List<String> *r_extensions) const
{
    if(!r_extensions->find(C11R_LANG_FILE_EXT))
    {
        r_extensions->push_back(C11R_LANG_FILE_EXT);
    }
}

bool ResourceFormatLoaderC11R::handles_type(const String &p_type) const
{
    return 	(p_type == "Script" || p_type == "C11RScript");
}

String ResourceFormatLoaderC11R::get_resource_type(const String &p_path) const
{
    String el = p_path.get_extension().to_lower();
	if (el == C11R_LANG_FILE_EXT) {
		return "C11RScript";
	}
	return "";
}

//
//  ResourceFormatSaverC11R
//

Error ResourceFormatSaverC11R::save(const String &p_path, const RES &p_resource, uint32_t p_flags)
{
    print_line("Saving C11R Script with custom format saver");
    const C11RScript* script = Object::cast_to<C11RScript>(*p_resource);
    if(script != NULL)
    {
        Ref<ConfigFile> cfg_file = script->save();
        if(cfg_file.is_valid())
        {
            return cfg_file->save(p_path);
        }
    }
    return Error::ERR_INVALID_DATA; // maybe there is a better error code?
}

bool ResourceFormatSaverC11R::recognize(const RES &p_resource) const
{
    return Object::cast_to<C11RScript>(*p_resource) != nullptr;
}

void ResourceFormatSaverC11R::get_recognized_extensions(const RES &p_resource, List<String> *r_extensions) const
{
    if(Object::cast_to<C11RScript>(*p_resource))
    {
        r_extensions->push_back(C11R_LANG_FILE_EXT);
    }
}