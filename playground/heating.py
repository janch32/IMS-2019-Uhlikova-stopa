import matplotlib.pyplot as plt 

def calcTemperature(ti, to, K, P, V, time):
    c = 1000
    m = 1.2 * V
    Qt = P * time
    Qz = K * (ti - to) * time

    return ti + (Qt - Qz) / (c * m)
    pass

time = range(0,3600*8)
temp = [20]

for i in time:
    temp.append(calcTemperature(temp[i], 30, 44, 0, 300, 1))

#for i in range(3600*3, 3600*6):
#    temp.append(calcTemperature(temp[i], 25, 250, 0, 300, 1))

temp.pop()

plt.plot(time, temp)
plt.xlabel('Čas (m)') 
plt.ylabel('Teplota')  
plt.title('Teplota místnosti') 
plt.show() 
