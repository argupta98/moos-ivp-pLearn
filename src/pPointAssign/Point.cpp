#include "Point.h"
#include <iostream>
#include <vector>
#include <cstdlib>
#include "MBUtils.h"

using namespace std;

Point::Point(string point_rep){
  point_string= point_rep;
  //now parse string into acctual coordiates
  //seperate string by commas
  vector<string> chomped= parseString(point_rep, ",");
  string x_string= chomped[0];
  string y_string= chomped[1];
  string id_string= chomped[2];
  biteStringX(x_string, '=');
  biteStringX(y_string, '=');
  biteStringX(id_string, '=');
  x_val = stoi(x_string);
  y_val= stoi(y_string);
  point_id = stoi(id_string);
  if(x_val<88){
    west=true;
  }
  else{
    west=false;
  }
}
  
  
