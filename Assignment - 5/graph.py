import io,matplotlib.pyplot as plt

file = 'test.txt'
title = 'Plot for page trace'

doc = io.open(file,'r',encoding='utf-8')
cont = doc.read().split('\n')
doc.close()
x,y = [],[]
i = 1
for word in cont[:-1] :
	a = int((word.split())[-1])
	if i%10 == 0:
		x.append(i)
		y.append(a)
	i+=1
plt.plot(x,y,color='blue',linestyle='dashed',linewidth=0,marker='o',markerfacecolor='blue',markersize=1)
plt.xlabel('Time / instruction no -->')
plt.ylabel('Page no')
plt.title(title)
plt.show()