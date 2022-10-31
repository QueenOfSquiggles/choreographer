#ifndef C11R_PARSER_H
#define C11R_PARSER_H

// refer to: https://docs.godotengine.org/en/stable/development/cpp/custom_resource_format_loaders.html

#include "core/io/resource_loader.h"
#include "core/io/resource_saver.h"

class ResourceFormatLoaderC11R : public ResourceFormatLoader
{
    GDCLASS(ResourceFormatLoaderC11R, ResourceFormatLoader);

public:
    virtual RES load(const String &path, const String &original_path, Error *r_error = NULL);
    virtual void get_recognized_extensions(List<String> *r_extensions) const;
    virtual bool handles_type(const String &p_type) const;
    virtual String get_resource_type(const String &p_path) const;

};

class ResourceFormatSaverC11R : public ResourceFormatSaver
{
    GDCLASS(ResourceFormatSaverC11R, ResourceFormatSaver);
public:
    virtual Error save(const String &p_path, const RES &p_resource, uint32_t p_flags = 0);
    virtual bool recognize(const RES &p_resource) const;
    virtual void get_recognized_extensions(const RES &p_resource, List<String> *r_extensions) const;
};

#endif // C11R_PARSER_H
