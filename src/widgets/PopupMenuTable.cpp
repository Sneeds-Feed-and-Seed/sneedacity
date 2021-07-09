/**********************************************************************

Sneedacity: A Digital Audio Editor

PopupMenuTable.cpp

Paul Licameli split from TrackPanel.cpp

**********************************************************************/


#include "PopupMenuTable.h"

PopupMenuTableEntry::~PopupMenuTableEntry()
{}

PopupSubMenu::~PopupSubMenu()
{}

PopupSubMenu::PopupSubMenu( const Identifier &stringId,
   const TranslatableString &caption_, PopupMenuTable &table_ )
   : ConcreteGroupItem< false >{ stringId }
   , WholeMenu{ caption_.empty() }
   , caption{ caption_ }
   , table{ table_ }
{
}

namespace {
struct PopupMenu : public wxMenu
{
   PopupMenu(wxEvtHandler *pParent_, void *pUserData_)
      : pParent{ pParent_ }, tables{}, pUserData{ pUserData_ } {}

   ~PopupMenu() override;

   void Extend(PopupMenuTable *pTable);
   void DisconnectTable(PopupMenuTable *pTable);
   void Disconnect();

   wxEvtHandler *pParent;
   std::vector<PopupMenuTable*> tables;
   void *pUserData;
};

class PopupMenuBuilder : public PopupMenuVisitor {
public:
   explicit
   PopupMenuBuilder( PopupMenuTable &table, PopupMenu &menu, void *pUserData )
      : PopupMenuVisitor{ table }
      , mMenu{ &menu }
      , mRoot{ mMenu }
      , mpUserData{ pUserData }
   {}

   void DoBeginGroup( Registry::GroupItem &item, const Path &path ) override;
   void DoEndGroup( Registry::GroupItem &item, const Path &path ) override;
   void DoVisit( Registry::SingleItem &item, const Path &path ) override;
   void DoSeparator() override;

   std::vector< std::unique_ptr<PopupMenu> > mMenus;
   PopupMenu *mMenu, *mRoot;
   void *const mpUserData;
};

void PopupMenuBuilder::DoBeginGroup( Registry::GroupItem &item, const Path &path )
{
   if ( auto pItem = dynamic_cast<PopupSubMenu*>(&item) ) {
      if ( !pItem->caption.empty() ) {
         auto newMenu =
            std::make_unique<PopupMenu>( mMenu->pParent, mMenu->pUserData );
         newMenu->tables.push_back( &pItem->table );
         mMenu = newMenu.get();
         mMenus.push_back( std::move( newMenu ) );
      }
   }
}

void PopupMenuBuilder::DoEndGroup( Registry::GroupItem &item, const Path &path )
{
   if ( auto pItem = dynamic_cast<PopupSubMenu*>(&item) ) {
      if ( !pItem->caption.empty() ) {
         auto subMenu = std::move( mMenus.back() );
         mMenus.pop_back();
         mMenu = mMenus.empty() ? mRoot : mMenus.back().get();
         mMenu->AppendSubMenu( subMenu.release(), pItem->caption.Translation());
      }
   }
}

void PopupMenuBuilder::DoVisit( Registry::SingleItem &item, const Path &path )
{
   auto pEntry = static_cast<PopupMenuTableEntry*>( &item );
   switch (pEntry->type) {
      case PopupMenuTable::Entry::Item:
      {
         mMenu->Append(pEntry->id, pEntry->caption.Translation());
         break;
      }
      case PopupMenuTable::Entry::RadioItem:
      {
         mMenu->AppendRadioItem(pEntry->id, pEntry->caption.Translation());
         break;
      }
      case PopupMenuTable::Entry::CheckItem:
      {
         mMenu->AppendCheckItem(pEntry->id, pEntry->caption.Translation());
         break;
      }
      default:
         wxASSERT( false );
         break;
   }

   // This call necessary for externally registered items, else harmlessly
   // redundant
   pEntry->handler.InitUserData( mpUserData );

   if ( pEntry->init )
      pEntry->init( pEntry->handler, *mMenu, pEntry->id );

   mMenu->pParent->Bind(
      wxEVT_COMMAND_MENU_SELECTED, pEntry->func, &pEntry->handler, pEntry->id);
}

void PopupMenuBuilder::DoSeparator()
{
   mMenu->AppendSeparator();
}

PopupMenu::~PopupMenu()
{
   // Event connections between the parent window and the singleton table
   // object must be broken when this menu is destroyed.
   Disconnect();
}
}

void PopupMenuTable::ExtendMenu( wxMenu &menu, PopupMenuTable &table )
{
   auto &theMenu = dynamic_cast<PopupMenu&>(menu);
   theMenu.tables.push_back( &table );

   PopupMenuBuilder visitor{ table, theMenu, theMenu.pUserData };
   Registry::Visit(
      visitor, table.Get( theMenu.pUserData ).get(), table.GetRegistry() );
}

void PopupMenuTable::RegisterItem(
   const Registry::Placement &placement, Registry::BaseItemPtr pItem )
{
   Registry::RegisterItem( *mRegistry, placement, std::move( pItem ) );
}

void PopupMenuTable::Append( Registry::BaseItemPtr pItem )
{
   mStack.back()->items.push_back( std::move( pItem ) );
}

void PopupMenuTable::Append(
   const Identifier &stringId, PopupMenuTableEntry::Type type, int id,
   const TranslatableString &string, wxCommandEventFunction memFn,
   const PopupMenuTableEntry::InitFunction &init )
{
   Append( std::make_unique<Entry>(
      stringId, type, id, string, memFn, *this, init ) );
}

void PopupMenuTable::BeginSection( const Identifier &name )
{
   auto uSection = std::make_unique< PopupMenuSection >( name );
   auto section = uSection.get();
   mStack.back()->items.push_back( std::move( uSection ) );
   mStack.push_back( section );
}

void PopupMenuTable::EndSection()
{
   mStack.pop_back();
}

namespace{
void PopupMenu::DisconnectTable(PopupMenuTable *pTable)
{
   class PopupMenuDestroyer : public PopupMenuVisitor {
   public:
      explicit
      PopupMenuDestroyer( PopupMenuTable &table, PopupMenu &menu )
         : PopupMenuVisitor{ table }
         , mMenu{ menu }
      {}

      void DoVisit( Registry::SingleItem &item, const Path &path ) override
      {
         auto pEntry = static_cast<PopupMenuTableEntry*>( &item );
         mMenu.pParent->Unbind( wxEVT_COMMAND_MENU_SELECTED,
         pEntry->func, &pEntry->handler, pEntry->id );
         pEntry->handler.DestroyMenu();
      }

      PopupMenu &mMenu;
   };

   PopupMenuDestroyer visitor{ *pTable, *this };
   Registry::Visit( visitor, pTable->Get( nullptr ).get(),
      pTable->GetRegistry() );
}

void PopupMenu::Disconnect()
{
   for ( auto pTable : tables )
      DisconnectTable(pTable);
}
}

// static
std::unique_ptr<wxMenu> PopupMenuTable::BuildMenu
( wxEvtHandler *pParent, PopupMenuTable *pTable, void *pUserData )
{
   // Rebuild as needed each time.  That makes it safe in case of language change.
   auto theMenu = std::make_unique<PopupMenu>( pParent, pUserData );
   ExtendMenu( *theMenu, *pTable );
   return theMenu;
}

