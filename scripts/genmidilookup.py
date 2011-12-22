midi=[0.0]*128
a=440.0
for i in range(0, 128):
	midi[i] = (a / 32.0) * (2.0 ** ((float(i) - 9.0) / 12.0));
	print "%f," % midi[i]