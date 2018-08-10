//Name: Arjun Gupta
//Date: June 19, 2017

#ifndef NODE_REPORT_DEFENSE_DEF
#define NODE_REPORT_DEFENSE_DEF

using namespace std;

class NodeReport{
 public: //define functions
  NodeReport(string node_report);
  ~NodeReport() {};
  void ProcessNodeReport(string node_report);
  string toString();
 public: 
  string name;
  string group;
  double nav_x;
  double nav_y;
  double heading;
  bool valid;
  bool tagged;
};

#endif
