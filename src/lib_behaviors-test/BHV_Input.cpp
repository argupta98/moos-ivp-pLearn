/************************************************************/
/*    NAME: Arjun Gupta                                     */
/*    ORGN: MIT                                             */
/*    FILE: BHV_Input.cpp                                   */
/*    DATE: NOvember 15 2017                                */
/************************************************************/

#include <iterator>
#include <cstdlib>
#include "MBUtils.h"
#include "BuildUtils.h"
#include "BHV_Input.h"
#include "ZAIC_PEAK.h"
#include "OF_Coupler.h"
#include "XYFormatUtilsPoly.h"
#include "XYFormatUtilsPoint.h"
#include <sstream>
#include <math.h>
#include <fstream>
#include <wchar.h>

using namespace std;
#define PI 3.14159265
//---------------------------------------------------------------
// Constructor

BHV_Input::BHV_Input(IvPDomain domain) :
  IvPBehavior(domain)
{
  m_bucket_size = 5;
  m_theta_size  = 15;
  m_blue_flag[0] = -58;
  m_blue_flag[1] = -71;
  m_red_flag[0]  =  50;
  m_red_flag[1] = -24;
  m_tagged= false;
  m_optimal = false;
  m_red_cap= "false";
  m_blue_cap= "false";
  m_heading_des=0;
  m_heading_abs=0;
  m_relative=false;
  m_module_loaded=true;
  m_infile = "table.csv";
  m_outfile = "BHV_Input.log";
  // Provide a default behavior name
  IvPBehavior::setParam("name", "InputBHV");

  // Declare the behavior decision space, only sets course and speed, not depth
  m_domain = subDomain(m_domain, "course,speed");

}

//-----------------------------------------------------------------
// Procedure: setup_python()
//   Purpose: loads the neural net object to store for future use and
//            starts the python interpreter

string BHV_Input::setup_python()
{
  vector<string> funcs;
  funcs.push_back("load");
  funcs.push_back("optimal_action");
  funcs.push_back("epsilon_greedy");
  string program = "call_keras";
  
  //process file data
  string players = setup_interpreter();

  //load models and place into pVal
  Py.setProgram(program);
  Py.addFuncs(funcs);
  pModelDict = Py.call("load", m_sVal, m_pVal, m_vVal);
  if(pModelDict){
    postMessage("INP_STAT", "loaded models");
    m_sVal.pop_back();
    m_pVal.push_back(pModelDict);
  }
  else{
    m_module_loaded = false;
  }
  return(players);
}

//-----------------------------------------------------------------
// Procedure: setup_interpreter()
//   Purpose: Reads in the configuration file, table.csv, sets up
//            state definition, action space, and other parameters
//            from the file. Also obtains the file location of the
//            neural net, and the names of other players in game. 
//   Returns: the list of opponents to keep track of 

string BHV_Input::setup_interpreter()
{
  //process file
  postMessage("INP_STAT", "setting up the interpreter");
  ifstream infile(m_infile);

  //Keep a file to write debugging info to
  ofstream outfile(m_outfile);
  outfile<<"beginning processing \n";
  string line="";
  string param, action;
  string out="";
  if (!infile.is_open()){
    outfile<<"Infile not found!!!\n";
    postWMessage("Cannot Find File");
  }
  else{
    while(getline(infile,line))
    {
      //divide in to variable and value
      outfile<<"read in: "+line+"\n";
      param=biteStringX(line, '=');
      
      //check for "relative" flag
      if(param=="relative"){
	if(line=="True"){
	  m_relative=true;
	  outfile<<"Using relative actionspace \n";
	}
      }

      //check for other players, add to output
      if(param=="players"){
	out+=",";
	vector<string> parsed_players = parseString(line, ',');
	for(int i=0; i<parsed_players.size(); i++){
	  NodeReport placeholder("invalid");
	  m_player_map.insert(make_pair(parsed_players[i], placeholder));
	  out+="NODE_REPORT_"+up(parsed_players[i]);
	}
      }
	
      //read in the length of state
      else if(param=="num_states"){
	m_state_size=atoi(line.c_str());
      }

      //find file location of model
      else if(param=="model_address"){
	outfile<<"wrote "<<line<<" to sVal\n";
	m_sVal.push_back(line);
      }

      //Define action space
      else if(param=="actions"){
	outfile<<"wrote "<<line<<" to vVal\n";
	vector<string> actions = parseString(line, ':');
	m_vVal.push_back(actions);
      }

      //Define whether we take optiomal actions or semi-random actions
      else if(param=="optimal"){
	if(line == "True"){
	  m_optimal = true;
	}
      }

      else if(param=="epsilon"){
	m_epsilon = line;
      }
      
      //check for and construct "state" definition
      else if(param=="state"){
	//format state= param1 type+heading object+flag index+0, param2...
	vector<string> params = parseString(line, ',');
	string param_name;
	int index=0;
	//go through "param: type+heading object+flag index+0"
	for(int i=0;i<params.size();i++){
	  vector<string> state_params(5,"None");
	  vector<string> fields = parseString(params[i], ' ');
	  //parse for the type, heading... object, flag pairs
	  for(int j=0; j<fields.size();j++){
	    vector<string> name_val = parseString(fields[j], '+');
	    if(name_val.size()==1){
	      param_name=name_val[0];
	    }
	    else if(name_val[0] == "type"){
	      state_params[0] = name_val[1];
	    }
	    else if(name_val[0] == "var"){
	      state_params[1] = name_val[1];
	    }
	    else if(name_val[0] == "var_mod"){
	      state_params[2] = name_val[1];
	    }
	    else if(name_val[0] == "bucket"){
	      state_params[3] = name_val[1];
	    }
	    else if(name_val[0] == "index"){
	      index = atoi(name_val[1].c_str());
	    }
	    else if(name_val[0] == "vehicle"){
	      state_params[4] = name_val[1];
	    }
	  }
	    //add state info into unordered maps for later consumption
	  m_state_indeces.insert(make_pair(param_name, index));
	  m_state_params.insert(make_pair(param_name, state_params));
	  outfile<<"made pair "+param_name+" and index "+to_string(index)+"\n";
	}
      }
    }
  }
  infile.close();
  outfile.close();
  return(out);
}

//---------------------------------------------------------------
// Procedure: setParam()

bool BHV_Input::setParam(string param, string val)
{
  // Convert the parameter to lower case for more general matching
  param = low(param);
  if(param == "infile"){
    m_infile = val;
    return(true);
  }
  return(false);
}

//---------------------------------------------------------------
// Procedure: onSetParamComplete()
//   Purpose: Invoked once after all parameters have been handled.
//            Good place to ensure all required params have are set.
//            Or any inter-param relationships like a<b.

void BHV_Input::onSetParamComplete()
{
  string players = setup_python();
  addInfoVars("NAV_X, NAV_Y, NAV_HEADING, TAGGED, TAGGED_VEHICLES, RED_FLAG_GRABBED, BLUE_FLAG_GRABBED, NODE_REPORT_LOCAL"+players);
}

//---------------------------------------------------------------
// Procedure: onHelmStart()
//   Purpose: Invoked once upon helm start, even if this behavior
//            is a template and not spawned at startup

void BHV_Input::onHelmStart()
{
}

//---------------------------------------------------------------
// Procedure: onIdleState()
//   Purpose: Invoked on each helm iteration if conditions not met.

void BHV_Input::onIdleState()
{
}

//---------------------------------------------------------------
// Procedure: onCompleteState()

void BHV_Input::onCompleteState()
{
}

//---------------------------------------------------------------
// Procedure: postConfigStatus()
//   Purpose: Invoked each time a param is dynamically changed

void BHV_Input::postConfigStatus()
{
}

//---------------------------------------------------------------
// Procedure: onIdleToRunState()
//   Purpose: Invoked once upon each transition from idle to run state

void BHV_Input::onIdleToRunState()
{
}

void BHV_Input::getVehicleData()
{
  bool succ;
  string node = getBufferStringVal("NODE_REPORT_LOCAL", succ);
  if(succ){
    NodeReport repr(node);
    m_name = repr.name;
    m_team = repr.group;
  }
  else
    postWMessage("Startup Node Report Not Recieved!!!");
}
  
//---------------------------------------------------------------
// Procedure: onRunToIdleState()
//   Purpose: Invoked once upon each transition from run to idle state

void BHV_Input::onRunToIdleState()
{
}

//---------------------------------------------------------------
// Procedure: onRunState()
//   Purpose: Invoked each iteration when run conditions have been met.

IvPFunction* BHV_Input::onRunState()
{
  // Part 1: Build the IvP function
  IvPFunction *ipf = 0;
  bool check1, check2, check3, check4, check5;

  //get vehicle info from node report
  if(m_name == ""){
    getVehicleData();
  }
  
  // Get Coordinates for self
  m_osX = getBufferDoubleVal("NAV_X", check1);
  m_osY = getBufferDoubleVal("NAV_Y", check2);
  m_heading_abs = getBufferDoubleVal("NAV_HEADING", check3);

  //get Coordinates for other vehicles
  string node;
  bool check_report;
  unordered_map<string, NodeReport>::iterator vehicle;
  for(vehicle = m_player_map.begin(); vehicle != m_player_map.end(); ++vehicle){
    node = getBufferStringVal("NODE_REPORT_"+up(vehicle->first), check_report);
    if(check_report){
      NodeReport repr(node);
      vehicle->second = repr;
    }
    else{
      string message = vehicle->first+" NodeReport not found!!";
      postWMessage(message);
    }
  }
  
  // Get Tagged Vehicles
  string tagged_vehicles = getBufferStringVal("TAGGED_VEHICLES", check3);
  if(check3){
    m_tagged_names = parseString(tagged_vehicles, ',');
    postWMessage(tagged_vehicles);
    for(int i=0; i<m_tagged_names.size();i++){
      postWMessage(m_tagged_names[i]);
      if(m_tagged_names[i] == m_name){
	      m_tagged=true;
      }
      else if(i == m_tagged_names.size()-1){
	      m_tagged=false;
      }
      unordered_map<string, NodeReport>::iterator veh = m_player_map.find(m_tagged_names[i]);
      if(veh != m_player_map.end()){
	      veh->second.tagged=true;
      }
    }
  }

  string tagged = getBufferStringVal("TAGGED", check3);
  if(check3){
    if(tagged == "true") m_tagged = true;
    else if(tagged == "false") m_tagged = false;
  }
  
     
  //Get Flag Taken Values
  m_red_cap = getBufferStringVal("RED_FLAG_GRABBED", check4);
  m_blue_cap = getBufferStringVal("BLUE_FLAG_GRABBED", check5);
  
  //descritize coordinate values and combine with other params to get state string
  string state = make_state();
  vector<string> inp = parseString(state, ',');

  //pass through neural net to get action
  if(m_module_loaded)
    make_action(inp); //sets speed and heading
  else{
    postWMessage("Module Not Loaded!!!");
    m_speed=2;
    m_heading_des=0;
  }

  //Convert to relative headings if flag set
  if(m_relative){
    m_heading_des=m_heading_abs+m_heading_des;
  }
  postMessage("INP_STAT", "state: "+state+" action: "+m_action);

  //build IvP function
  ipf = buildFunctionWithZAIC();
  
  // Part N: Prior to returning the IvP function, apply the priority wt
  // Actual weight applied may be some value different than the configured
  // m_priority_wt, depending on the behavior author's insite.
  if(ipf)
    ipf->setPWT(m_priority_wt);

  return(ipf);
}


//---------------------------------------------------------------
// Procedure: make action()
// Purpose: Pass through the state to the neural net and pick optimal actions
// output an action as "(speed, heading)"

void BHV_Input::make_action(vector<string> state)
{
  //load in state parameter 
  m_vVal.push_back(state);
  for(int i = 0; i< m_pVal.size(); ++i){
    cout << m_pVal[i];
  }
  
  //call appropriate function from python
  if(m_pVal[0] != NULL){
    if(m_optimal)
      pResult = Py.call("optimal_action", m_sVal, m_pVal, m_vVal);
    else{
      m_sVal.push_back(m_epsilon);
      pResult = Py.call("epsilon_greedy", m_sVal, m_pVal, m_vVal);
      m_sVal.pop_back();
    }
    //remove state parameter
    m_vVal.pop_back();
  }
  else{
    postWMessage("Lost model");
  }

  //get result and parse information
  string temp = PyBytes_AsString(pResult);
  temp.erase(temp.begin());
  temp.erase(temp.end()-1);
  vector<string> action = parseString(temp, ',');
  string speed = action[0];
  string heading = action[1];
  heading.erase(heading.begin());
  m_action = speed+","+heading;
  m_speed=atof(speed.c_str());
  m_heading_des=atof(heading.c_str());
  postMessage("INP_STAT", "successfully reset action!");
}

//---------------------------------------------------------------
// Procedure: make state()
// Purpose: construct state representation to be passed through the neural net
// Output: String containing the state

string BHV_Input::make_state()
{
  int my_flag[2];
  int enemy_flag[2];
  
  //get proper flag coordinates
  if(m_team=="blue"){
    my_flag[0]=m_blue_flag[0];
    my_flag[1]=m_blue_flag[1];
    enemy_flag[0]=m_red_flag[0];
    enemy_flag[1]=m_red_flag[1];
  }
  else{
    my_flag[0]=m_red_flag[0];
    my_flag[1]=m_red_flag[1];
    enemy_flag[0]=m_blue_flag[0];
    enemy_flag[1]=m_blue_flag[1];
  }

  int index;
  vector<string> parameters;
  double obj=0;
  vector<double> state(m_state_size, 0.0);
  string vehicle,type,var,var_mod;
  int bucket;
    
  //go through dictionary of state_params and construct state
  unordered_map<string, int>::iterator i;
  for(i = m_state_indeces.begin(); i != m_state_indeces.end(); ++i){
    //Extract info for state
    parameters=m_state_params[i->first];
    index=i->second;
    type=parameters[0];
    var=parameters[1];
    var_mod=parameters[2];
    bucket = atoi(parameters[3].c_str());
    vehicle = parameters[4];

    //process binary features
    if(type == "binary"){
      if(var == "team"){
	if(m_team=="blue"){
	  obj=1.0;
	}
	else{
	  obj=0.0;
	}
      }
      else if(var == "flag_captured"){
	obj=process_flag_captured();
      }
      else if(var == "tagged"){
	obj=process_tagged();
      }
    }

    //process distance features
    else if(type =="distance"){
      if(vehicle=="self"){
	//distance to self or enemy flag
	if(var == "flag"){
	  if(var_mod=="self"){
	    obj=process_dist(my_flag[0], my_flag[1], bucket, m_osX, m_osY);
	    showPoint("my flag", "green", my_flag[0], my_flag[1]);
	  }
	  else if(var_mod=="enemy"){
	    obj=process_dist(enemy_flag[0], enemy_flag[1], bucket,  m_osX, m_osY);
	    showPoint("enemy flag", "green", enemy_flag[0], enemy_flag[1]);
	  }
	}
	//distance to bounds
	else if(var == "leftBound"){
	  obj = processBound(m_osX, m_osY, "left");
	}
	else if(var == "rightBound"){
	  obj = processBound(m_osX, m_osY, "right");
	}
	else if(var == "upperBound"){
	  obj = processBound(m_osX, m_osY, "up");
	}
	else if(var == "lowerBound"){
	  obj = processBound(m_osX, m_osY, "down");
	}
      }
      else{
	 unordered_map<string, NodeReport>::iterator veh = m_player_map.find(vehicle);
	 if(veh != m_player_map.end()){
	   if(var=="flag"){
	     if(var_mod=="self"){
	       obj=process_dist(my_flag[0], my_flag[1], bucket, veh->second.nav_x, veh->second.nav_y);
	       
	     }
	     else if(var_mod=="enemy"){
	       obj=process_dist(enemy_flag[0], enemy_flag[1], bucket, veh->second.nav_x, veh->second.nav_y);
	     }
	   }
	   else if(var=="player"){
	     showPoint("enemy", "red", veh->second.nav_x,  veh->second.nav_y);
	     obj=process_dist(m_osX, m_osY, bucket, veh->second.nav_x, veh->second.nav_y);
	   }
	 }
	 else{
	   string message = vehicle+" not specified in Constants.py";
	   postWMessage(message);
	 }
      }
    }
    
    else if(type=="angle"){
      //process angular features
      if(vehicle == "self"){
	if(var =="flag"){
	  if(var_mod=="self"){
	    obj=process_angle(my_flag[0], my_flag[1], bucket, "absolute", m_osX, m_osY);
	  }
	  else if(var_mod=="enemy"){
	    obj=process_angle(enemy_flag[0], enemy_flag[1], bucket, "absolute", m_osX, m_osY);
	  }
	}
      }
      else{
	 unordered_map<string, NodeReport>::iterator veh = m_player_map.find(vehicle);
	 if(veh != m_player_map.end()){
	   if(var=="flag"){
	     if(var_mod=="self"){
	       obj=process_angle(my_flag[0], my_flag[1], bucket, "absolute",veh->second.nav_x, veh->second.nav_y);
	     }
	     else if(var_mod=="enemy"){
	       obj=process_angle(enemy_flag[0], enemy_flag[1], bucket, "absolute",veh->second.nav_x, veh->second.nav_y);
	     }
	   }
	   else if(var=="player"){
	     obj=process_angle(m_osX, m_osY, bucket, "absolute", veh->second.nav_x, veh->second.nav_y);
	   }
	 }
	 else{
	   string message = vehicle+" not specified in Constants.py";
	   postWMessage(message);
	 }
      }
    }
    
    else if(parameters[0]=="raw"){
      if(vehicle == "self"){
	//process raw data
	int bucket=atoi(parameters[3].c_str());
	if(parameters[1]=="x"){
	  obj=m_osX/bucket;
	}
	else if(parameters[1]=="y"){
	  obj=m_osY/bucket;
	}
	else if(parameters[1]=="heading"){
	  obj=m_heading_abs/bucket;
	}
      //add in support for raw enemy parameters
      }
      else{
	unordered_map<string, NodeReport>::iterator veh = m_player_map.find(vehicle);
	if(veh != m_player_map.end()){
	  int bucket=atoi(parameters[3].c_str());
	  if(parameters[1]=="x"){
	    obj=veh->second.nav_x/bucket;
	  }
	  else if(parameters[1]=="y"){
	    obj=veh->second.nav_y/bucket;
	  }
	  else if(parameters[1]=="heading"){
	    obj=veh->second.heading/bucket;
	  }
	}
	else{
	  string message = vehicle+" not specified in Constants.py";
	  postWMessage(message);
	}
      }
      
    }
    else{
      postWMessage("VAR NOT FOUND ERROR");
    }
    state[index]=obj;
  }


  //process state into a usable string
  string state_string="";
  for(int i=0;i<state.size();i++){
    ostringstream converter;
    converter << state[i];
    string conv(converter.str());
    state_string+=conv;
    if(i != state.size()-1){
      state_string+=",";
    }
  }

  //make a grab request if close enough to flag
  double dist = process_dist(enemy_flag[0], enemy_flag[1], 1, m_osX, m_osY);
  if(dist < 10){
    string message = "vname="+m_name;
    postMessage("FLAG_GRAB_REQUEST", message);
  }
  
  return(state_string);
}

//---------------------------------------------------------------
// Helper Functions for make_state() to handle each case

double BHV_Input::process_dist(double goal_x, double goal_y, int bucket_size, double x, double y)
{
  double dist = sqrt(pow(x-goal_x,2)+pow(y-goal_y,2));
  return(dist/bucket_size);
}

double BHV_Input::process_angle(double goal_x, double goal_y, int bucket_size, string flags, double x, double y)
{
  double theta = atan2((y-goal_y),(x-goal_x))*180/PI;
  if(theta<0){
    theta=360+theta;
  }
  if(flags == "relative"){
    theta=90+m_heading_abs-theta;
    if(theta<0){
      theta=360+theta;
    }
  }
  return(theta/bucket_size);
}
  
double BHV_Input::process_flag_captured()
{
  int out=0;
  if(m_team=="blue"){
    if(m_red_cap=="true"){
      out=1;
    }
  }
  else{
    if(m_blue_cap=="true"){
      out=1;
    }
  }
  return(out);
}


double BHV_Input::process_tagged()
{
  bool out_of_bounds=false;
  int out=0;
  int x=m_osX+53;
  int y=m_osY+114;
  //rotate -24.775 degrees to make correspond to flat rectangle
  double x_proj=x*.908+y*.41906;
  double y_proj=-x*.41906+y*.908;

  //check bounds
  if(x_proj<0 || x_proj>146 || y_proj<0 || y_proj>71){
    out_of_bounds=true;
  }
 
  if(m_tagged || out_of_bounds){
    out=1;
    postWMessage("tagged!");
  }
  return(out);
}

double BHV_Input::processBound(double x, double y, string side){
  //Define corners for use
  double upper_left_corner[2] = {-83, -49};
  double upper_right_corner[2] = {56, 16};
  double lower_left_corner[2] = {-53, -114};
  double lower_right_corner[2] = {82, -56};
  
  double a_vehicle, b_vehicle, c_vehicle;
  double a_bound, b_bound, c_bound;
  
  //make line ax+by = c for vehicle
  if(side == "left" || side == "right"){
    a_vehicle = -65; //the y part of the slope
    b_vehicle = 139; //the x part of the slope
    c_vehicle = a_vehicle*x+b_vehicle*y;
    a_bound = 65;
    b_bound = 30;
  }
  
  else{ //we are looking at a vertical boundary
    a_vehicle = 65; //the y part of the slope
    b_vehicle = 30; //the x part of the slope
    c_vehicle = a_vehicle*x+b_vehicle*y;
    a_bound = -65;
    b_bound = 139;
  }

  //set boundary slopes as well
  //make line ax+by = c for boundary
  if(side == "left"){
    a_bound = upper_left_corner[1]-lower_left_corner[1];
    b_bound = -(upper_left_corner[0]-lower_left_corner[0]);
    c_bound = a_bound*(upper_left_corner[0])+b_bound*(upper_left_corner[1]);
  }

  else if(side == "up"){
    a_bound = upper_left_corner[1]-upper_right_corner[1];
    b_bound = -(upper_left_corner[0]-upper_right_corner[0]);
    c_bound = a_bound*(upper_left_corner[0])+b_bound*(upper_left_corner[1]);
  }
  else if(side == "right"){
    a_bound = upper_right_corner[1]-lower_right_corner[1];
    b_bound = -(upper_right_corner[0]-lower_right_corner[0]);
    c_bound = a_bound*(upper_right_corner[0])+b_bound*(upper_right_corner[1]);
  }
  else if(side == "down"){
    a_bound = lower_left_corner[1]-lower_right_corner[1];
    b_bound = -(lower_left_corner[0]-lower_right_corner[0]);
    c_bound = a_bound*(lower_left_corner[0])+b_bound*(lower_left_corner[1]);
  }
  else{
    postWMessage("Incorrect Arguments for processBound");
  }


  //now find intersection point
  double determinant = a_vehicle*b_bound-a_bound*b_vehicle;

  if(determinant == 0){
    postWMessage("0 determinant!!!");
    return 0;
  }
  //solve system of linear equations
  else{
    double x_intercept = (b_bound*c_vehicle - c_bound*b_vehicle)/determinant;
    double y_intercept = (a_vehicle*c_bound - a_bound*c_vehicle)/determinant;
    //now get distance from current position to that point on boundary
    
    //Display the point in the simulation
    showPoint(side+" boundary", "dodger_blue", x_intercept, y_intercept);
    
    /*
    XYSegList boundary_line;
    boundary_line.add_vertex(x_intercept, y_intercept, 0, "");
    boundary_line.add_vertex(x, y, 0, "");
    boundary_line.set_edge_size(1);
    boundary_line.set_edge_color("doger_blue");
    postMessage("VIEW_SEGLIST", boundary_line.get_spec(), side+" bound");
    */
    return process_dist(x_intercept, y_intercept, 1, x, y);
  }

}

//---------------------------------------------------------------
// Procedure: showPoint()
//   Purpose: show the given point in pMarineViewer

void BHV_Input::showPoint(string name, string color, double x, double y)
{
    XYPoint point;
    point.set_label(name);
    point.set_vertex_size(3);
    point.set_color("vertex", color);
    point.set_color("label", color);
    point.set_vertex(x, y);
    postMessage("VIEW_POINT", point.get_spec("active=true"), name);
}

//---------------------------------------------------------------
// Procedure: buildFunctionWithZAIC()
//     Purpose: Builds and IvP function for the behavior

IvPFunction *BHV_Input::buildFunctionWithZAIC()
{
  ZAIC_PEAK spd_zaic(m_domain, "speed");
  spd_zaic.setSummit(m_speed);
  spd_zaic.setBaseWidth(0.3);
  spd_zaic.setPeakWidth(0.0);
  spd_zaic.setSummitDelta(0.0);
  IvPFunction *spd_of = spd_zaic.extractIvPFunction();

  ZAIC_PEAK crs_zaic(m_domain, "course");
  crs_zaic.setSummit(m_heading_des);
  crs_zaic.setBaseWidth(180.0);
  crs_zaic.setValueWrap(true);
  IvPFunction *crs_of = crs_zaic.extractIvPFunction();

  OF_Coupler coupler;
  IvPFunction *ipf = coupler.couple(crs_of, spd_of);

  return(ipf);
}


//----------------------------------------------------------------
// Procedure: up() and low()
//   Purpose: convert to uper case or lowercase
//   rewritten due to issue with Python.cpp library overrides

string BHV_Input::up(string orig){
  string new_str = orig;
  for (int i=0; i<new_str.length(); i++)
  {
    if (new_str[i]>='a' && new_str[i]<='z')
    {
       new_str[i]=new_str[i]-'a'+'A';
    }
  }
  return new_str;
}

string BHV_Input::low(string orig){
  string new_str = orig;
  for (int i=0; i<new_str.length(); i++)
  {
    if (new_str[i]>='a' && new_str[i]<='z')
    {
       new_str[i]=new_str[i]+'a'-'A';
    }
  }
  return new_str;
}
