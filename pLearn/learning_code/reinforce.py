
"""
Created on Sat Nov 18 9:43 2017

@author: Arjun

File to implement reinforcement learning algorithm
"""

from DeepLearn import Deep_Learner
from Constants import Constants
import math
import sys
import matplotlib.pyplot as plt
import subprocess
   
Constants=Constants()

def make_states():
    """
    function to make all possible states, works for arbitrarily many
    states defined in Constants.py
    """
    
    params=[0]*Constants.num_states
    for param in Constants.state:
        temp=[]
        if Constants.state[param].type == "binary":
            temp=[0,1]
        elif Constants.state[param].type == "distance" or Constants.state[param].type =="angle" or Constants.state[param].type=="raw":
            beg=Constants.state[param].range[0]
            end=Constants.state[param].range[1]
            step=Constants.state[param].bucket
            for i in range(beg, end/step+1):
                temp.append(i)
        else:
            raise SyntaxError("Incorrect Definition of state parameter "+param+" in Constants.py")
        
        params[Constants.state[param].index]=temp

    #enumerate all possible states
    all_states=[[param] for param in params[0]]
    index=1
    while index<len(params):
       new_states=[]
       for state in all_states:
          for param in params[index]:
             new_states.append(state+[param])
       all_states=new_states
       index+=1

    for index in range(len(all_states)):
       all_states[index]=tuple(all_states[index])
       
    return all_states


def make_actions():
    """
    how to descritize action space (using step). Create different action spaces dependent 
    on Constant flags. 
    """
    all_actions=[]        
    if(not Constants.relative):
        for i in range(0, 360, Constants.theta_size_act):
            for speed in Constants.speeds:
                all_actions.append((speed, i))
    else:
        for i in Constants.rel_headings:
            for speed in Constants.speeds:
                all_actions.append((speed, i))
    return all_actions
    
def make_rewards(smooth = True):
    """
    make reward function. This part is very important to making the 
    model work correctly. Due to sparse rewards, try giving scalled down
    rewards some distance from the actual reward point. Also negative 
    rewards for going out of bounds
    """
    if smooth:
        def reward_fn(state):
            enemy_dist=state[Constants.state["enemy_dist"].index]
            if(enemy_dist < 10):
                return Constants.neg_reward
            else:
                return within_goal(state)+within_bound(state)+within_tagged(state)
    else:
        def reward_fn(state):
            #check within bounds (first priority)
            boundary_dist=min(state[Constants.state["leftBound"].index],
                            state[Constants.state["rightBound"].index],
                            state[Constants.state["upperBound"].index],
                            state[Constants.state["lowerBound"].index])
            if(boundary_dist < 5):
                return Constants.neg_reward

            enemy_dist=state[Constants.state["enemy_dist"].index]
            if(enemy_dist < 5):
                return Constants.neg_reward

            dist=state[Constants.state["flag_dist"].index]
            discount = Constants.reward_dropoff
            return Constants.max_reward*discount**(max(0, dist))
            
            
        
    return reward_fn

    
def within_goal(state):
   """
   helper function to give a reward based on proximity to the goal state (flag)
   """
   reward_dropoff=Constants.reward_dropoff
   dist=state[Constants.state["flag_dist"].index]
   if state[Constants.state["out"].index] != 1:
        value = min(max(dist-10, 0), 200)
        return float(Constants.max_reward) - value*(float(Constants.max_reward)/200)
   else:
        return 0 

def within_bound(state):
   """
   helper function to give a negative reward based on proximity to the boundary
   """
   boundary_dist=min(state[Constants.state["leftBound"].index],
            state[Constants.state["rightBound"].index],
            state[Constants.state["upperBound"].index],
            state[Constants.state["lowerBound"].index])

   if boundary_dist == state[Constants.state["leftBound"].index]:
       if boundary_dist < 5:
           return -1.5*Constants.max_reward-boundary_dist*(-1.5*Constants.max_reward/10)
       else:
           return 0
       
   elif boundary_dist < 20:
       return Constants.neg_reward-boundary_dist*(Constants.neg_reward/20)
   
   else:
       return 0

def within_tagged(state):
   """
   helper function to give a reward based on proximity to the goal state (flag)
   """
   enemy_dist=state[Constants.state["enemy_dist"].index]
   if state[Constants.state["out"].index] == 1:
       return Constants.neg_reward
   elif enemy_dist < 10:
       return Constants.neg_reward
   elif enemy_dist < 30:
       return Constants.neg_reward-enemy_dist*(Constants.neg_reward/30)
   else:
       return 0
    
def evaluate(deep_learner, process_cmd, simulation_cmd, iters = Constants.num_eval_iters,
            process_path = Constants.process_path, read_path = Constants.read_path):
    """
    function to interface with the simulation_engine (moos simulator). Calls subfunction
    that runs and processes moos_data
    """
    #set up trajectory and open a file to write the training data to for checking/reloading later
    total_trials = 0
    trials_out = 0
    trials_tagged = 0
    flag_captured = 0 
    trial_num = 0
    flag_loc = [-58, -71]
    print("starting simulation")
    for _ in range(iters):
        trial_num+=1
        #call simulation
        subprocess.call([simulation_cmd, str(trial_num)])
        
        #process simulation.alog with log_converter.py
        process_file=process_path+'/simulation_'+str(trial_num)+'/simulation.alog'
        read_file=read_path+'/simulation_'+str(trial_num)+'.alog'
        cmd=['python',process_cmd,process_file,read_file]
        print("processing data from", process_file, "using", process_cmd,"to", read_file)
        subprocess.call(cmd)
        data = []

        with open(read_file, 'r') as inp:
            #split into lines and store in array
            data=inp.read().splitlines()

        for line in data:
            (x,y,tagged,out) = line.split()
            x = float(x)
            y = float(y)
            total_trials += 1
            if(tagged == "True"): 
                trials_tagged+=1
                print("\n\nTAGGED!!!\n\n")
                break
            if(out == "True"):
                trials_out+=1
                print("\n\nOUT!!!\n\n")
                break
            d_x = x-flag_loc[0]
            d_y = y-flag_loc[1]
            distance_to_flag = (d_x**2 + d_y**2)**(.5)
            if(distance_to_flag < 8):
                print("\n\nFlag Captured!\n\n")
                flag_captured+=1
                break

    pct_capture = float(flag_captured)/trial_num
    pct_tagged = float(trials_tagged)/trial_num
    pct_out = float(trials_out)/trial_num
    print(str(pct_out) + " " + str(pct_capture))

    return (pct_out, pct_capture, pct_tagged)
#----------------------------------------------------------------------
#                            Main
#----------------------------------------------------------------------

   
if(sys.argv[1]=="new" or sys.argv[1]=="debug" or sys.argv[1]=="test" or sys.argv[1]=="evaluate"):
    learner=Deep_Learner(make_rewards(Constants.smooth_reward), make_actions(), Constants.alg_type, False)
elif(sys.argv[1]=="load"):
    learner=Deep_Learner(make_rewards(Constants.smooth_reward), make_actions(), Constants.alg_type, True)
else:
    raise SyntaxError("Not a Valid Argument, Please call with on of the following flags: 'test', 'load', 'new', 'evaluate'")
    
if(sys.argv[1] == "new" or sys.argv[1] == "load"):
    learner.initialize()
    if(Constants.alg_type == "fitted"):
        learner.fitted_Q_learn()
    else:
        learner.DQL()
        
elif(sys.argv[1]=="test"):
    if len(sys.argv)>2:
        learner.initialize()
        performance_metrics=[]
        iters = int(sys.argv[2])
        out1 = open(Constants.test_address+"pre_assessment.log", 'w')
        process = "/home/arjun/moos-ivp-argupta/pLearn/learning_code/baseline_constructor.py"
        simulation = "/home/arjun/moos-ivp-argupta/pLearn/learning_code/baseline_evaluator.sh"
        for i in range(iters+1):
            learner.output_table(optimal = True, model_address = Constants.test_address+"iteration_"+str(i)+"/")
            percent_inbound, score, percent_captured = learner.sim_episode(0, iters=Constants.num_test_iters, table=False, simulation_cmd = Constants.test_sim_cmd)
            pct_in, _ = evaluate(learner, process, simulation, iters = 1)
            percent_inbound = percent_inbound-(1-pct_in)
            performance_metrics.append((i, int(percent_inbound*100), int(score), int(percent_captured*100)))
            #print out progress
            out1.write("\n iteration_"+str(i)+":  avg reward: "+str(score)+" pct time inbounds: "+str(percent_inbound)+
                            " pct captured: "+str(percent_captured)+"\n")
            print("\n------ Finished evaluating iteration "+str(i)+": "+str(int(percent_inbound*100))+" pct time inbounds,  "+str(score)+" reward -----\n")

        out1.close()
         #plot performance
        fig = plt.figure(1)

        plt.subplot(222)
        plt.title("Percent Time Inbounds and Untagged")
        plt.scatter([x[0] for x in performance_metrics], [x[1] for x in performance_metrics])
        plt.plot([x[0] for x in performance_metrics], [x[1] for x in performance_metrics], color = 'blue')

        plt.subplot(212)
        plt.title("Reward")
        plt.scatter([x[0] for x in performance_metrics], [x[2] for x in performance_metrics])
        plt.plot([x[0] for x in performance_metrics], [x[2] for x in performance_metrics], color = 'red') 
        
        plt.subplot(221)
        plt.title("Percent of Successful Captures")
        plt.scatter([x[0] for x in performance_metrics], [x[3] for x in performance_metrics])
        plt.plot([x[0] for x in performance_metrics], [x[3] for x in performance_metrics], color = 'green') 

        fig.savefig(Constants.test_address+"model_graph.png")
        #sort the models by reward
        performance_metrics.sort(key = lambda x: x[2])
        
        #write models out to a file

        outfile = open(Constants.test_address+"model_assessment.log", 'w')

        #sort top models by percent time inbounds, from highest to lowest
        #performance_metrics.sort(key = lambda x: -x[1])
        outfile.write("\n----------------Top Performers--------------------\n\n")
        for i, percent_inbound, score, percent_captured in performance_metrics:
            outfile.write("\n iteration_"+str(i)+":  avg reward: "+str(score)+" pct time inbounds: "+str(percent_inbound)+
                            " pct captured: "+str(percent_captured)+"\n")
        
        outfile.close()

        for r in range(15):
            (i, _, _, _) = performance_metrics[r]
            learner.output_table(optimal = True, model_address = Constants.test_address+"iteration_"+str(i)+"/")
            percent_inbound, score, percent_captured = learner.sim_episode(0, iters=Constants.num_test_iters, table=False, simulation_cmd = Constants.test_sim_cmd)
            pct_in, _ = evaluate(learner, process, simulation)
            percent_inbound = percent_inbound-(1-pct_in)
            save_address = Constants.test_address+"iteration_"+str(i)+"/evaluation_statistics.log"
            outfile = open(save_address, 'w')
            outfile.write("\n------- Ran "+str(Constants.num_eval_iters)+" iterations -----\n\n")
            outfile.write("avg reward: "+str(score)+" pct time inbounds: "+str(percent_inbound)+
                                    " pct captured: "+str(percent_captured)+"\n")
            outfile.close()

        plt.show()
            
        
    else:
        learner.output_table(optimal=True,  model_address = Constants.test_address)
        

elif sys.argv[1] == "evaluate":
    save_address = Constants.test_address+"evaluation_statistics.log"
    base_address = "/home/arjun/moos-ivp-argupta/pLearn/learning_code/"
    process = base_address+"baseline_constructor.py"
    simulation = base_address+"evaluator.sh"
    if(len(sys.argv) > 2):
        simulation = base_address+"baseline_evaluator.sh"
        save_address = base_address + "baseline_evaluation.log"
    learner.output_table(optimal = True, model_address = Constants.test_address)
    pct_out, percent_captured, percent_tagged = evaluate(learner, process, simulation)
    outfile = open(save_address, 'w')
    outfile.write("\n------- Ran "+str(Constants.num_eval_iters)+" iterations -----\n\n")
    outfile.write(" pct time out: "+str(pct_out)+
                  " pct captured: "+str(percent_captured)+
                  " pct time tagged: "+str(percent_tagged)+"\n")
    outfile.close()
