#include "register_types.h"

C11RScriptLanguage* c11r_script_language = nullptr;

void register_c11r_types()
{
    c11r_script_language = memnew(C11RScriptLanguage);
    ScriptServer::register_language(c11r_script_language);
}

void unregister_c11r_types()
{
    ScriptServer::unregister_language(c11r_script_language);
    if(c11r_script_language)
    {
        memdelete(c11r_script_language);
    }
}
