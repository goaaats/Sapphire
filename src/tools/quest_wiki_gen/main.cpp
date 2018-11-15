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

std::string titleCase( const std::string& str )
{
  if( str.empty() )
    return str;

  std::string retStr( str );
  std::transform( str.begin(), str.end(), retStr.begin(), ::tolower );
  std::locale loc;
  retStr[ 0 ] = std::toupper( str[ 0 ], loc );
  for( size_t i = 1; i < str.size(); ++i )
  {
    if( str[ i - 1 ] == ' ' || str[ i - 1 ] == '_' ||
        ( std::isdigit( str[ i - 1 ], loc ) && !std::isdigit( str[ i ], loc ) ) )
      retStr[ i ] = std::toupper( str[ i ], loc );
  }
  return retStr;
}

/*
void
createScript( std::shared_ptr< Core::Data::Quest >& pQuestData, std::set< std::string >& additionalList, int questId )
{
  std::string header(
    "// This is an automatically generated C++ script template\n"
    "// Content needs to be added by hand to make it function\n"
    "// In order for this script to be loaded, change its extension to .cpp\n"
    "// and move it to the correct folder in <root>/scripts/native/\n"
    "\n"
    "#include <ScriptObject.h>\n\n"
  );

  std::size_t splitPos( pQuestData->id.find( "_" ) );
  std::string className( pQuestData->id.substr( 0, splitPos ) );
  //className = "Quest" + className;
  std::string sceneStr(
    "   //////////////////////////////////////////////////////////////////////\n   // Available Scenes in this quest, not necessarly all are used\n" );
  std::string seqStr;
  seqStr.reserve( 0xFFF );
  seqStr += ( "      // Steps in this quest ( 0 is before accepting, \n      // 1 is first, 255 means ready for turning it in\n" );
  std::string questVarStr( "      // Quest vars / flags used\n" );

  seqStr += "      enum Sequence : uint8_t\n      {\n";
  for( auto& entry : additionalList )
  {
    if( entry.find( "OnScene" ) != std::string::npos )
    {
      std::string sceneName = entry.substr( 2 );
      std::string sceneId = entry.substr( 7 );

      std::size_t numOff = sceneId.find_first_not_of( "0" );
      sceneId = numOff != std::string::npos ? sceneId.substr( numOff ) : "0";

      sceneStr += std::string(
        "   void " +
        sceneName +
        "( Entity::Player& player )\n"
        "   {\n"
        "      player.eventPlay( this->getId(), " +
        sceneId +
        ", 0,\n"
        "         [&]( Entity::Player& player, uint32_t eventId, uint16_t param1, uint16_t param2, uint16_t param3 )\n"
        "         {\n"
        "         });\n"
        "   }\n\n"
      );
    }
    else if( entry.find( "Flag" ) != std::string::npos ||
             entry.find( "QuestUI" ) != std::string::npos )
    {
      questVarStr += "      // " + entry + "\n";
    }
    else if( entry.find( "SEQ" ) != std::string::npos )
    {
      if( entry.find( "SEQ_FINISH" ) != std::string::npos )
      {
        seqStr += "         SeqFinish = 255,\n";
      }
      else if( entry.find( "SEQ_OFFER" ) != std::string::npos )
      {
      }
      else
      {
        std::string seqName = titleCase( entry );

        seqName.erase( std::remove_if( seqName.begin(), seqName.end(), []( const char c ) {
          return c == '_';
        }), seqName.end());

        std::string seqId = entry.substr( 4 );
        seqStr += "         " + seqName + " = " + seqId + ",\n";
      }
    }
  }
  seqStr += "      };\n";
  std::string rewards;
  rewards.reserve( 0xFFF );
  rewards += "      // Quest rewards \n";
  rewards += ( pQuestData->expFactor != 0 ) ? "      static constexpr auto RewardExpFactor = " +
                                              std::to_string( pQuestData->expFactor ) + ";\n" : "";
  rewards += ( pQuestData->gilReward != 0 ) ? "      static constexpr auto RewardGil = " +
                                              std::to_string( pQuestData->gilReward ) + ";\n" : "";
  rewards += ( pQuestData->emoteReward != 0 ) ? "      static constexpr auto RewardEmote = " +
                                                std::to_string( pQuestData->emoteReward ) + ";\n" : "";
  rewards += ( pQuestData->actionReward != 0 ) ? "      static constexpr auto RewardAction = " +
                                                 std::to_string( pQuestData->actionReward ) + ";\n" : "";
  rewards += ( pQuestData->generalActionReward[ 0 ] != 0 ) ? "      static constexpr auto RewardGeneralAction1 = " +
                                                             std::to_string( pQuestData->generalActionReward[ 0 ] ) +
                                                             ";\n" : "";
  rewards += ( pQuestData->generalActionReward[ 1 ] != 0 ) ? "      static constexpr auto RewardGeneralAction2 = " +
                                                             std::to_string( pQuestData->generalActionReward[ 1 ] ) +
                                                             ";\n" : "";
  rewards += ( pQuestData->gCSeals != 0 ) ? "      static constexpr auto RewardGCSeals = " +
                                            std::to_string( pQuestData->gCSeals ) + ";\n" : "";
  rewards += ( pQuestData->otherReward != 0 ) ? "       static constexpr auto RewardOther = " +
                                                std::to_string( pQuestData->otherReward ) + ";\n" : "";
  rewards += ( pQuestData->reputationReward != 0 ) ? "      static constexpr auto RewardReputation = " +
                                                     std::to_string( pQuestData->reputationReward ) + ";\n" : "";
  rewards += ( pQuestData->tomestoneReward != 0 ) ? "      static constexpr auto RewardTomeType = " +
                                                    std::to_string( pQuestData->tomestoneReward ) + ";\n" : "";
  rewards += ( pQuestData->tomestoneCountReward != 0 ) ? "      static constexpr auto RewardTomeCount = " +
                                                         std::to_string( pQuestData->tomestoneCountReward ) + ";\n"
                                                       : "";
  rewards += ( pQuestData->instanceContentUnlock != 0 ) ? "      static constexpr auto InstancedContentUnlock = " +
                                                          std::to_string( pQuestData->instanceContentUnlock ) + ";\n"
                                                        : "";

  if( !pQuestData->itemReward0.empty() )
  {
    rewards += "      static constexpr auto RewardItem[] = { ";
    for( size_t ca = 0; ca < pQuestData->itemReward0.size(); ca++ )
    {
      rewards += std::to_string( pQuestData->itemReward0.at( ca ) );
      if( ca != pQuestData->itemReward0.size() - 1 )
      {
        rewards += ", ";
      }
    }
    rewards += " };\n";
  }

  if( !pQuestData->itemReward0.empty() )
  {
    rewards += "      static constexpr auto RewardItemCount[] = { ";
    for( size_t ca = 0; ca < pQuestData->itemCountReward0.size(); ca++ )
    {
      rewards += std::to_string( pQuestData->itemCountReward0.at( ca ) );
      if( ca != pQuestData->itemCountReward0.size() - 1 )
      {
        rewards += ", ";
      }
    }
    rewards += " };\n";
  }

  if( !pQuestData->itemReward1.empty() )
  {
    rewards += "      static constexpr auto RewardItemOptional[] = { ";
    for( size_t ca = 0; ca < pQuestData->itemReward1.size(); ca++ )
    {
      rewards += std::to_string( pQuestData->itemReward1.at( ca ) );
      if( ca != pQuestData->itemReward1.size() - 1 )
      {
        rewards += ", ";
      }
    }
    rewards += " };\n";
  }

  if( !pQuestData->itemCountReward1.empty() )
  {
    rewards += "      static constexpr auto RewardItemOptionalCount[] = { ";
    for( size_t ca = 0; ca < pQuestData->itemCountReward1.size(); ca++ )
    {
      rewards += std::to_string( pQuestData->itemCountReward1.at( ca ) );
      if( ca != pQuestData->itemCountReward1.size() - 1 )
      {
        rewards += ", ";
      }
    }
    rewards += " };\n";
  }

  bool hasERange = false;
  bool hasEmote = false;
  bool hasEnemies = false;
  std::vector< uint32_t > enemy_ids;
  std::vector< std::string > script_entities;
  std::string sentities = "      // Entities found in the script data of the quest\n";

  for( size_t ca = 0; ca < pQuestData->scriptInstruction.size(); ca++ )
  {
    if( ( pQuestData->scriptInstruction.at( ca ).find( "HOWTO" ) != std::string::npos ) ||
        ( pQuestData->scriptInstruction.at( ca ).find( "HOW_TO" ) != std::string::npos ) )
      continue;

    if( ( pQuestData->scriptInstruction.at( ca ).find( "EMOTENO" ) != std::string::npos ) ||
        ( pQuestData->scriptInstruction.at( ca ).find( "EMOTEOK" ) != std::string::npos ) )
      hasEmote = true;

    if( pQuestData->scriptInstruction.at( ca ).find( "ENEMY" ) != std::string::npos )
    {
      hasEnemies = true;
      enemy_ids.push_back( pQuestData->scriptArg.at( ca ) );
    }

    if( !pQuestData->scriptInstruction.at( ca ).empty() )
      script_entities.push_back(
        pQuestData->scriptInstruction.at( ca ) + " = " + std::to_string( pQuestData->scriptArg.at( ca ) ) );
  }
  std::sort( script_entities.begin(), script_entities.end() );
  for( auto& entity : script_entities )
  {
    auto name = titleCase( entity );

    name.erase( std::remove_if( name.begin(), name.end(), []( const char c ) {
      return c == '_';
    }), name.end());

    sentities += "      static constexpr auto " + name + ";\n";
  }

  std::string additional = "// Quest Script: " + pQuestData->id + "\n";
  additional += "// Quest Name: " + pQuestData->name + "\n";
  additional += "// Quest ID: " + std::to_string( questId ) + "\n";
  additional += "// Start NPC: " + std::to_string( pQuestData->eNpcResidentStart ) + "\n";
  additional += "// End NPC: " + std::to_string( pQuestData->eNpcResidentEnd ) + "\n\n";

  std::string scriptEntry;
  scriptEntry.reserve( 0xFFFF );
  scriptEntry += "   //////////////////////////////////////////////////////////////////////\n   // Event Handlers\n";

  scriptEntry += onTalkStr;

  if( hasERange )
  {
    scriptEntry += onWithinRangeStr;
  }

  if( hasEmote )
  {
    scriptEntry += onEmoteStr;
  }

  for( auto enemy : enemy_ids )
  {
    scriptEntry += std::string(
      "   void onMobKill_" + std::to_string( enemy ) + "( Entity::Player& player )\n"
                                                       "   {\n"
                                                       "   }\n\n"
    );
  }

  std::string constructor;
  constructor += std::string(
    "   private:\n"
    "      // Basic quest information \n" );
  constructor += questVarStr + "\n";
  constructor += seqStr + "\n";
  constructor += rewards + "\n";
  constructor += sentities + "\n";
  constructor += "   public:\n";
  constructor += "      " + className + "() : EventScript" + "( " + std::to_string( questId ) + " ){}; \n";
  constructor += "      ~" + className + "(){}; \n";

  std::string classString(
    "class " + className + " : public EventScript\n"
                           "{\n" +
    constructor +
    "\n" +
    scriptEntry +
    "   private:\n" +
    sceneStr +
    "};\n\n"
  );

  std::ofstream outputFile;

  outputFile.open( "generated/" + className + ".cpp" );
  outputFile << header << additional << classString;
  outputFile.close();
}
*/

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
    std::cout << "Usage: quest_parser \"path/to/ffxiv/game/sqpack\" <1/0 unluac export toggle>\n";
    return 0;
  }

  xiv::dat::GameData data( datLocation );

  auto rows = g_exdDataGen.getQuestIdList();

  if( !fs::exists( "./wiki" ) )
    fs::create_directory( "./wiki" );

  g_log.info( "Export in progress" );

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
