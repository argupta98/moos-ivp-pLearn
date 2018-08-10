import sys, os

print(sys.version)
print(sys.path)

try:
    import time
    import random
except Exception as e:
    print(e)
    
try:
    import numpy as np
    print(np.version)
except Exception as e:
    print(e)

try:
    from keras.models import Sequential
    from keras.layers.core import Dense
    from keras.models import load_model
except Exception as e:
    print(e)

try:
    from Constants import Constants
except Exception as e:
    print(e)

Constants = Constants()

def load(path, actions):
    model_dict = {}
    print("loading actions "),
    if(Constants.alg_type == "fitted"):
        for action in actions:
            print('.'),
            model_dict[action]=load_model(path+action+".h5")
    else:
        model_dict["model"] = load_model(path+"model.h5")
    print("done")
    return model_dict

def optimal_action(model_dict, actions, s):
     if(model_dict == None or actions == None or s == None):
         print("NULL MODEL")
         return actions[0]
     
     if(Constants.alg_type == "fitted"):
         opt=(0,None)
         for action in actions:
             act_val=model_dict[action].predict(state2vec(s))
             if act_val >= opt[0] or opt[1]==None:
                 opt=(act_val, action)
         return opt[1]
     
     else:
         opt = actions[np.argmax(model_dict["model"].predict(state2vec(s))[0])]
         return opt
    
    
def epsilon_greedy(model_dict, epsilon, actions, s):
    print("epsilon greedy")
    if(model_dict == None):
         print("NULL MODEL")
         return actions[0]
    epsilon = float(epsilon)
    print("Epsilon = "+str(epsilon))
    if np.random.random_sample() < epsilon:
        print("taking a random action sample");
        return random.sample(actions, 1)[0]
    print("returning optimal action");
    return optimal_action(model_dict, actions, s)

def state2vec(s):
    print("state2vec")
    if(s==None):
        print("NULL STATE")
    temp = []
    print("Preprocessed State: "+str(s))
    for state in s:
        temp.append(float(state))
    temp.append(1)
    for param in Constants.state:
        print("on "+param)
        if Constants.state[param].standardized:
            if Constants.state[param].type != "binary":
                temp[Constants.state[param].index]=float(int(temp[Constants.state[param].index])-Constants.state[param].range[0])/Constants.state[param].range[1]
    print("state2Vec finishing with value: "+str(temp))
    return np.array([temp])
