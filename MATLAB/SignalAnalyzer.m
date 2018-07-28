Serial = serial('COM12', 'BaudRate', 115200);

fopen(Serial);

i = 0;
while(1)
    val = fscanf(Serial);
    plot(i, i, )
    i = i+1;
end
