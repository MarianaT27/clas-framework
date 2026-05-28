/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

#include "dictionary.h"
#include "utils.h"
#include <cstdlib>

namespace hipo {

  void  schema::parse(const std::string& schString){
    std::vector<std::string>  entries;
    std::vector<std::string>  entry;
    hipo::utils::tokenize(schString, entries, ",");
    int offset = 0;
    for(size_t i = 0; i < entries.size(); i++){
      entry.clear();
      hipo::utils::tokenize(entries[i],entry, "/");
      schemaEntry_t e;
      e.name = hipo::utils::trim(entry[0]);
      std::string raw = hipo::utils::trim(entry[1]);
      // Split optional "#N" array-length suffix: e.g. "F#6" -> type "F", length 6.
      auto hash = raw.find('#');
      if(hash == std::string::npos){
        e.type = raw;
        e.arrayLength = 1;
      } else {
        e.type = raw.substr(0, hash);
        hipo::utils::trim(e.type);
        std::string len_str = raw.substr(hash + 1);
        hipo::utils::trim(len_str);
        int len = std::atoi(len_str.c_str());
        if(len <= 0){
          throw hipo::schema_error(
            std::string("hipo::schema: invalid array length for column '") +
            e.name + "' in schema '" + schemaName + "' (must be > 0)");
        }
        e.arrayLength = len;
      }
      e.typeId   = getTypeByString(e.type);
      e.typeSize = getTypeSize(e.typeId);
      e.offset   = offset;
      offset    += e.typeSize * e.arrayLength;
      schemaEntries.push_back(e);
      schemaEntriesMap[e.name] = i;
   }
  }

  int   schema::getTypeByString(std::string &typeName){
      if(typeName=="B"){
        return kByte;
      } else if(typeName=="S") {
        return kShort;
      } else if(typeName=="I") {
        return kInt;
      } else if(typeName=="F") {
        return kFloat;
      } else if(typeName=="D") {
        return kDouble;
      } else if(typeName=="L") {
        return kLong;
      }
      return -1;
  }

  int  schema::getTypeSize(int id){
    switch(id){
      case kByte:   return 1;
      case kShort:  return 2;
      case kInt:    return 4;
      case kFloat:  return 4;
      case kDouble: return 8;
      case kLong:   return 8;
      default:      return 0;
    }
    return 0;
  }

  void  schema::show(){
    printf("schema : %14s , group = %6d, item = %3d\n",
       schemaName.c_str(),groupid,itemid);
    for(auto & schemaEntrie : schemaEntries){
      printf("%16s : (%3s) %5d %5d , offset = %3d , length = %3d --> [%s]\n",
         schemaEntrie.name.c_str(),schemaEntrie.type.c_str(),
         schemaEntrie.typeId,schemaEntrie.typeSize, schemaEntrie.offset,
         schemaEntrie.arrayLength,
         schemaEntrie.name.c_str()
         );
    }
  }


  int   schema::getSizeForRows(int rows){
    int nentries = schemaEntries.size();
    const auto &last = schemaEntries[nentries-1];
    int offset   = getOffset(nentries-1,rows-1,rows) + last.typeSize * last.arrayLength;
    return offset;
  }

  std::string  schema::getSchemaString(){
    char parts[256];
    std::string result;
    snprintf(parts,256,"{%s/%d/%d}{",schemaName.c_str(),groupid,itemid);
    result.append(parts);
    for(size_t loop = 0; loop < schemaEntries.size(); loop++){
      const auto &e = schemaEntries[loop];
      if(e.arrayLength > 1){
        snprintf(parts,256,"%s/%s#%d", e.name.c_str(), e.type.c_str(), e.arrayLength);
      } else {
        snprintf(parts,256,"%s/%s", e.name.c_str(), e.type.c_str());
      }
      if(loop!=0) result.append(",");
      result.append(parts);
    }
    result.append("}");
    return result;
  }

  std::string  schema::getSchemaStringJson(){
     char parts[256];
    std::string result;
    snprintf(parts,256,"{ \"name\": \"%s\", \"group\": %d, \"item\": %d, \"info\": \" \",",
               schemaName.c_str(),groupid,itemid);
    result.append(parts);
    result.append("\"entries\": [ ");
    for(size_t loop = 0; loop < schemaEntries.size(); loop++){
      const auto &e = schemaEntries[loop];
      if(e.arrayLength > 1){
        snprintf(parts,256,"{\"name\":\"%s\", \"type\":\"%s\", \"length\":%d, \"info\":\" \"}",
         e.name.c_str(), e.type.c_str(), e.arrayLength);
      } else {
        snprintf(parts,256,"{\"name\":\"%s\", \"type\":\"%s\", \"info\":\" \"}",
         e.name.c_str(), e.type.c_str());
      }
      if(loop!=0) result.append(",");
      result.append(parts);
    }
    result.append("] }");
    return result;
  }


  //=============================================
  // Implementation of dictionary class
  //=============================================
  std::vector<std::string> dictionary::getSchemaList(){
    std::map<std::string, schema>::iterator it;
    std::vector<std::string> vec;
    for ( it = factory.begin(); it != factory.end(); it++ ){
        vec.push_back(it->first);
    }
    return vec;
  }

  bool    dictionary::parse(const char *schemaString){
    std::vector<std::string> tokens;
    std::string schemahead = hipo::utils::substring(schemaString,"{","}",0);
    hipo::utils::tokenize(schemahead, tokens, "/");
    int group = std::atoi(tokens[1].c_str());
    int  item = std::atoi(tokens[2].c_str());
    hipo::schema  schema (tokens[0].c_str(),group,item);
    std::string schemabody = hipo::utils::substring(schemaString,"{","}",1);
    schema.parse(schemabody.c_str());
    addSchema(schema);
    return true;
  }

  void    dictionary::show(){
    std::vector<std::string> list = getSchemaList();
    for(auto & i : list){
      schema sc = getSchema(i.c_str());
      printf("%24s : %5d %5d %5d\n", sc.getName().c_str(),
        sc.getGroup(), sc.getItem(),sc.getEntries());
    }
  }
}
