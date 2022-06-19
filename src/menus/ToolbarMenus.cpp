

#include "../Menus.h"
#include "../ProjectSettings.h"
#include "../commands/CommandContext.h"
#include "../commands/CommandManager.h"
#include "../toolbars/ToolManager.h"
#include "../Debug.h"

/// Namespace for functions for View Toolbar menu
namespace ToolbarActions {
// exported helper functions
// none
// Menu handler functions
struct Handler : CommandHandlerObject {
void OnResetToolBars(const CommandContext &context)
{
   dprintf("menus/ToolbarMenus.cpp: void OnResetToolBars(const CommandContext &context)");
   ToolManager::OnResetToolBars(context);
}
}; // struct Handler
} // namespace

static CommandHandlerObject &findCommandHandler(SneedacityProject &) {
   dprintf("menus/ToolbarMenus.cpp: static CommandHandlerObject &findCommandHandler(SneedacityProject &)");
   // Handler is not stateful.  Doesn't need a factory registered with
   // SneedacityProject.
   static ToolbarActions::Handler instance;
   return instance;
};

// Menu definitions
#define FN(X) (& ToolbarActions::Handler :: X)
namespace{
using namespace MenuTable;
BaseItemSharedPtr ToolbarsMenu()
{
   dprintf("menus/ToolbarMenus.cpp: BaseItemSharedPtr ToolbarsMenu()");
   using Options = CommandManager::Options;
   static BaseItemSharedPtr menu{
   ( FinderScope{ findCommandHandler },
   Section( wxT("Toolbars"),
      Menu( wxT("Toolbars"), XXO("&Toolbars"),
         Section( "Reset",
            /* i18n-hint: (verb)*/
            Command( wxT("ResetToolbars"), XXO("Reset Toolb&ars"),
               FN(OnResetToolBars), AlwaysEnabledFlag )
         ),
         Section( "Other" )
      )
   ) ) };
   return menu;
}

AttachedItem sAttachment1{
   Placement{ wxT("View/Other"), { OrderingHint::Begin } },
   Shared( ToolbarsMenu() )
};
}

#undef FN
