#include "register_types.h"
#include "core/engine.h"

#include "c11r_lang.hpp"
#include "editor/c11r_editor.hpp"
#include "packs/block_pack.hpp"
#include "parser/c11r_parser.hpp"


C11RScriptLanguage* c11r_script_language = nullptr;

#ifdef TOOLS_ENABLED
static _C11REditor *c11r_editor_singleton = nullptr;
#endif

static Ref<ResourceFormatLoaderC11R> loader_c11r;
static Ref<ResourceFormatSaverC11R> saver_c11r;

void register_c11r_types()
{
    c11r_script_language = memnew(C11RScriptLanguage);
    ScriptServer::register_language(c11r_script_language);
	
	ClassDB::register_class<C11RScript>();
	ClassDB::register_virtual_class<Block>();
	ClassDB::register_class<BlockPack>();


	#ifdef TOOLS_ENABLED
		ClassDB::set_current_api(ClassDB::API_EDITOR);
		{ // API_EDITOR registration
			ClassDB::register_class<_C11REditor>();
			ClassDB::register_class<C11REditor>();
		}
		ClassDB::set_current_api(ClassDB::API_CORE);

		c11r_editor_singleton = memnew(_C11REditor);
		Engine::get_singleton()->add_singleton(Engine::Singleton("C11REditor", _C11REditor::get_singleton()));

		C11REditor::register_editor();
	#endif

	loader_c11r.instance();
	ResourceLoader::add_resource_format_loader(loader_c11r);

	saver_c11r.instance();
	ResourceSaver::add_resource_format_saver(saver_c11r);
	
}

void unregister_c11r_types()
{
    ScriptServer::unregister_language(c11r_script_language);
	
	#ifdef TOOLS_ENABLED
	C11REditor::free_clipboard();
	if(c11r_editor_singleton)
	{
		memdelete(c11r_editor_singleton);
	}
	#endif

    if(c11r_script_language)
    {
        memdelete(c11r_script_language);
    }

	ResourceLoader::remove_resource_format_loader(loader_c11r);
	loader_c11r.unref();

	ResourceSaver::remove_resource_format_saver(saver_c11r);
	saver_c11r.unref();

}