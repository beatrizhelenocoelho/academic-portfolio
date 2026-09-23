
Utils for lab1:

mymodbus.m	wrapper to work for Matlab before and after 2017a

myterminal3.m	handles keyboard keys (columns 194:196, lines 184:187)
			in YOUR program use columns 14:16, lines 4:7


-- Use of myterminal3.m in YOUR unity program:

inputs
%m0 	presence switch
%m1	alarm switch
%m2	window switch
%m4-%m7	keyboard lines (need columns power, duration after each press ~1sec)

outputs
%m10	buzzer
%m11	Red LED
%m12	Yellow LED
%m13	Green LED
%m14-%m16 actuate keyboard columns
