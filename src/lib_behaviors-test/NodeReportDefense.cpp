//Name: Arjun Gupta
//Date: June 19, 2017

#include "MBUtils.h"
#include <vector>
#include "NodeReportDefense.h"
#include <stdlib.h>

using namespace std;

void NodeReport::ProcessNodeReport(string node_report){
  valid = false;
  vector<string> elements= parseString(node_report, ',');
  for(int i=0; i<elements.size(); i++){
    string value = elements[i];
    string key= biteStringX(value, '=');
    if(key == "NAME"){
      name = value;
    }
    else if(key == "GROUP"){
      group = value;
    }
    else if(key == "X"){
      nav_x = atof(value.c_str());
    }
    else if(key == "Y"){
      nav_y = atof(value.c_str());
    }
    else if(key =="HDG"){
      heading = atof(value.c_str());
    }
  }
}

NodeReport::NodeReport(string node_report){
  valid=true;
  tagged=false;
  name="blah";
  nav_x=0;
  nav_y=0;
  ProcessNodeReport(node_report);
}

string NodeReport::toString(){
  string output="Node Report with Name= ";
  output+=name;//+", NAV_X= "+to_string(nav_x)+", NAV_Y= "+to_string(nav_y)+ " process_message= "+to_string(valid);
  return output;
}
