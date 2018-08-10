//Name: Arjun Gupta
//Date: June 19, 2017

#ifndef NODE_REPORT_DEF
#define NODE_REPORT_DEF

class NodeReport{
 public: //define functions
  NodeReport(string node_report);
  ~NodeReport() {};
  void ParseNodeReport(string node_report);
 public: 
  string name;
  string group;
  double nav_x;
  double nav_y;
  double heading;
  bool valid;
};

#endif
