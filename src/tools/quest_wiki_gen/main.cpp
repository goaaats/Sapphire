#include <cstdint>
#include <fstream>
#include <iostream>
#include <locale>
#include <set>
#include <string>
#include <cstring>

#include <experimental/filesystem>

#include <Exd.h>
#include <ExdCat.h>
#include <ExdData.h>
#include <File.h>
#include <GameData.h>
#include <DatCat.h>

#include <Exd/ExdDataGenerated.h>
#include <Logging/Logger.h>

#include <algorithm>


Core::Logger g_log;
Core::Data::ExdDataGenerated g_exdDataGen;
namespace fs = std::experimental::filesystem;

struct QuestEntry
{
  int rowId;
  std::shared_ptr< Core::Data::Quest > questPtr;
};

std::string getFormattedName(std::string name)
{
  std::replace( name.begin(), name.end(), ' ', '-');

  return name;
}

const std::string specialScriptRequirement[] { "INSTANCEDUNGEON", "QUESTBATTLE", "ENEMY", "MOUNT", "STATUS" };

std::string checkIsScriptable(QuestEntry quest)
{
  std::string requirements( "" );

  for( int i = 0; i < quest.questPtr->scriptInstruction.size(); i++ )
  {
    auto instruction = quest.questPtr->scriptInstruction[i];

    if (instruction.rfind("INSTANCEDUNGEON", 0) == 0)
      requirements += "Instance Content(" + std::to_string( quest.questPtr->scriptArg[i] ) + "),";

    if (instruction.rfind("CONTENT_FINDER_CONDITION", 0) == 0)
      requirements += "Content(" + std::to_string( quest.questPtr->scriptArg[i] ) + "),";

    if (instruction.rfind("QUESTBATTLE", 0) == 0)
      requirements += "Quest Battle(" + std::to_string( quest.questPtr->scriptArg[i] ) + "),";

    if (instruction.rfind("ENEMY", 0) == 0)
      requirements += "Battle NPC(" + std::to_string( quest.questPtr->scriptArg[i] ) + "),";

    if (instruction.rfind("MOUNT", 0) == 0)
      requirements += "Mount(" + std::to_string( quest.questPtr->scriptArg[i] ) + "),";

    if (instruction.rfind("STATUS", 0) == 0)
      requirements += "Status Effect(" + std::to_string( quest.questPtr->scriptArg[i] ) + "),";
  }

  if(!requirements.empty())
    requirements = requirements.substr( 0, requirements.size() - 1 );

  return requirements;
}

int main( int argc, char** argv )
{

  g_log.init();

  std::string datLocation( "C:/SquareEnix/FINAL FANTASY XIV - A Realm Reborn/game/sqpack" );
  if( argc > 1 )
    datLocation = std::string( argv[ 1 ] );

  g_log.info( "Setting up generated EXD data" );
  if( !g_exdDataGen.init( datLocation ) )
  {
    std::cout << datLocation << "\n";
    g_log.fatal( "Error setting up EXD data " );
    std::cout << "Usage: quest_wiki_gen \"path/to/ffxiv/game/sqpack\"\n";
    return 0;
  }

  xiv::dat::GameData data( datLocation );

  auto rows = g_exdDataGen.getQuestIdList();

  if( !fs::exists( "./wiki" ) )
    fs::create_directory( "./wiki" );

  g_log.info( "Generating..." );

  std::vector< std::string > scannedQuests;

  for( auto it = std::experimental::filesystem::recursive_directory_iterator( "..\\..\\servers\\Scripts\\quest" ); it != std::experimental::filesystem::recursive_directory_iterator(); ++it )
  {
    if(it->path().filename().extension().string() != ".cpp")
      continue;

    scannedQuests.push_back( it->path().filename().stem().string() );
  }

  uint32_t updateInterval = rows.size() / 20;
  uint32_t i = 0;
  auto typeMap = std::map< int, std::vector< QuestEntry > >();
  auto typeAmountMap = std::map<int, int>();
  auto typeCompletedMap = std::map<int, int>();

  for( const auto& row : rows )
  {
    auto questInfo = g_exdDataGen.get< Core::Data::Quest >( row );

    auto entry = QuestEntry();
    entry.rowId = row;
    entry.questPtr = questInfo;

    typeMap[questInfo->journalGenre].push_back( entry );

    ++i;
    if( i % updateInterval == 0 )
      std::cout << ".";
  }

  auto typeCompletionMap = std::map<int, float>();

  std::string indexHeader(
    "# Quest Scripting Overview\n\nThis is a overview of all quests in the game and if they are already scripted.\n\nIf you find that a quest is scripted and is not working or has problems, please [create a issue](../../issues/new).\n\n\n\n| Name | Percentage Done | Amount Done |\n| ------------- |:-------------:| ----- |\n"
  );

  std::string allQuestsHeader(
    "# Quests: All Quests\n\nThis is a overview of all quests in the game and if they are already scripted.\n\nIf you find that a quest is scripted and is not working or has problems, please [create a issue](../../issues/new).\n\n\n\n| S | Name | ID | Class Requirements | Restrictions |\n| ------------- | ------------- | ----- | --- | --- |\n"
  );

  time_t _tm =time(NULL );
  struct tm * curtime = localtime ( &_tm );
  std::string time = std::string( asctime(curtime) );
  time = time.substr( 0, time.length() - 1 );
  
  std::string footer(
    "\n\n---\n*generated by quest_wiki_gen on " + time + "*"
  );

  auto journalGenreRows = g_exdDataGen.getJournalGenreIdList();

  int allAmount = 0;
  int allCompleted = 0;

  std::string allQuestList( "" );
  std::string genreList( "" );
  for( const auto& row : journalGenreRows )
  {
    auto genreInfo = g_exdDataGen.get< Core::Data::JournalGenre >( row );

    std::string questListHeader(
    "# Quests: " + genreInfo->name + "\n\nThis is a overview of all \"" + genreInfo->name + "\" quests in the game and if they are already scripted.\n\nIf you find that a quest is scripted and is not working or has problems, please [create a issue](../../issues/new).\n\n\n\n| S | Name | ID | Class Requirements | Restrictions |\n| ------------- | ------------- | ----- | --- | --- |\n"
    );

    std::string questList( "" );
    for( const auto& questInfo : typeMap[row] )
    {
      std::size_t splitPos( questInfo.questPtr->id.find( "_" ) );
      std::string className( questInfo.questPtr->id.substr( 0, splitPos ) );

      typeAmountMap[questInfo.questPtr->journalGenre]++;

      bool completed = false;
      if( std::find( scannedQuests.begin(), scannedQuests.end(), className ) != scannedQuests.end() )
        completed = true;

      if(completed)
        typeCompletedMap[questInfo.questPtr->journalGenre]++;

      std::string classReq( "" );

      if( questInfo.questPtr->classJobCategory0 != 0 )
        classReq += g_exdDataGen.get< Core::Data::ClassJobCategory >( questInfo.questPtr->classJobCategory0 )->name + "(Lv" + std::to_string( questInfo.questPtr->classJobLevel0 ) + ")";

      if( questInfo.questPtr->classJobCategory0 != 0 && questInfo.questPtr->classJobCategory1 != 0 )
        classReq += " / ";

      if( questInfo.questPtr->classJobCategory1 != 0 )
        classReq += g_exdDataGen.get< Core::Data::ClassJobCategory >( questInfo.questPtr->classJobCategory1 )->name + "(Lv" + std::to_string( questInfo.questPtr->classJobLevel1 ) + ")";

      auto scriptableRestriction = checkIsScriptable( questInfo );

      std::string status = std::string( completed ? "✔️" : "❌" );

      if(!scriptableRestriction.empty() && !completed)
        status = "❓";

      std::string questString = "| " + status + " | [" + questInfo.questPtr->name + "](http://www.garlandtools.org/db/#quest/" + std::to_string( questInfo.rowId ) + ") | " + className + "(" + std::to_string( questInfo.rowId ) + ") | " + classReq + " | " + scriptableRestriction + " |\n";
      questList += questString;
      allQuestList += questString;
    }
     
    if( genreInfo->name.empty() )
      continue;

    float amount = 0;
    if( typeAmountMap.count( row ) )
      amount = (float) typeAmountMap[row];

    if( amount == 0 )
      continue;

    float completed = 0;
    if( typeCompletedMap.count( row ) )
      completed = (float) typeCompletedMap[row];

    allAmount += amount;
    allCompleted += completed;
    
    std::ofstream out("wiki\\" + getFormattedName( genreInfo->name ) + "(" + std::to_string( row ) + ")" + ".md");
    out << questListHeader + questList + footer;
    out.close();

    genreList += "| [" + genreInfo->name +"](" + getFormattedName( genreInfo->name ) + "(" + std::to_string( row ) + ")" + ") | " + std::to_string( ( completed / amount ) * 100.0 ).substr( 0, 4 ) + "% | " + std::to_string(typeCompletedMap[row]) + "/" + std::to_string(typeAmountMap[row]) + " |\n";
  }

  genreList += "| **[All Quests](All-Quests)** | **" + std::to_string( ( allCompleted / allAmount ) * 100.0 ).substr( 0, 4 ) + "%** | **" + std::to_string( allCompleted ) + "/" + std::to_string( allAmount ) + "** |\n";

  std::ofstream allQuestOut("wiki\\All-Quests.md");
  allQuestOut << allQuestsHeader + allQuestList + footer;
  allQuestOut.close();

  std::ofstream indexOut("wiki\\Quest-Index.md");
  indexOut << indexHeader + genreList + footer;
  indexOut.close();

  std::cout << "\nDone!";
  return 0;
}