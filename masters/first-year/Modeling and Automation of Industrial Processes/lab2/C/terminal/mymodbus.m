function varargout = mymodbus( varargin )
%
% Function wrapper to allow Matlab versions both before and after 2017a
% 
% m= mymodbus('ini');
% mymodbus('read', m, target, address, count);
% mymodbus('write', m, target, address, values);
% mymodbus('end', m);
% 
% Note: in Matlab versions < 2017a every read/write implies 'end' and 'ini'
% it is a bug / incomplete modbus protocol implementation

% Mar2020, JG

if verLessThan('matlab','9.2') %0 %1
    [varargout{1:nargout}]= mymodbus_m16( varargin{:} );
else
    [varargout{1:nargout}]= mymodbus_m17( varargin{:} );
end


% ======================================================================
function ret= debug_level( op, a1 )
% debug_level('get')
persistent dbLevel
ret= [];
switch op
    case 'ini', dbLevel= 0;
    case 'set', dbLevel= a1;
    case 'get', ret= dbLevel;
    otherwise, error('inv op');
end


% ======================================================================
function ret= mymodbus_m17( cmd, a1, a2, a3, a4 )
% Modbus for Matlab versions >= 2017a (i.e. ones that have modbus.m)

switch cmd
    case 'ini', debug_level('ini'); ret= modbus_ini;
    case 'end' % do nothing
    case 'read', ret= myread( a1, a2, a3, a4 ); % m, target, address, count
    case 'write', mywrite( a1, a2, a3, a4 ); % m, target, address, values
    case 'db_level_set', debug_level('set', a1);
    otherwise
        error('inv cmd')
end


function m= modbus_ini
m= modbus('tcpip', '127.0.0.1', 502);


function ret= myread(m, target, address, count)
% target is 'coils' or 'holdingregs'
% address: 1x1 : memory address 0,1,2,...

ret= read(m, target, address+1, count); % modbus uses 1,2... instead of 0,1...

if debug_level('get')==1
    if length(ret)==1
        fprintf(1, 'read target=%s address=%d ret=%d\n', target, address, ret);
    else
        fprintf(1, 'read target=%s address1=%d len(ret)=%d\n', target, address(1), length(ret));
    end
end


function mywrite(m, target, address, values)
% target is 'coils' or 'holdingregs'
% address: 1x1 : memory address 0,1,2,...

write(m,target,address+1,values); % modbus uses 1,2... instead of 0,1...

if debug_level('get')==1
    if length(values)==1
        fprintf(1, 'write target=%s address=%d value=%d\n', target, address, values);
    else
        fprintf(1, 'write target=%s address=%d len(values)=%d\n', target,address,length(values));
    end
end


% ======================================================================
function ret= mymodbus_m16( cmd, a1, a2, a3, a4 )
%
% Modbus use with Matlab versions before 2017a

% Mar2020, JG

switch cmd
    case 'ini', debug_level('ini'); ret= modbus_ini0; % ret == m
    case 'end', fclose( a1 ); % a1 = m
    case 'read', ret= myread0( a1, a2, a3, a4 ); % m, target, address, count
    case 'write', mywrite0( a1, a2, a3, a4 ); % m, target, address, values
    case 'db_level_set', debug_level('set', a1);
    otherwise
        error('inv cmd')
end


% ----------------------------------------------------------------------
function ret= myread0(m, target, address, count)
%ret= read(m, target, address+1, count); % modbus uses 1,2... instead of 0,1...

if strcmp(target, 'coils')
    ret= Modbus1( m, address, count ); % tcpip_pipe, Address, nBits0
elseif strcmp(target, 'holdingregs')
    %error('only "coils" implemented till now');
    ret= Modbus3( m, address, count );
else
    error('inv target');
end

if debug_level('get')==1
    if length(ret)==1
        fprintf(1, 'read target=%s address=%d ret=%d\n', target, address, ret);
    else
        fprintf(1, 'read target=%s address1=%d len(ret)=%d\n', target, address(1), length(ret));
    end
end


function mywrite0(m, target, address, values)
%write(m,target,address+1,values); % modbus uses 1,2... instead of 0,1...

if strcmp(target, 'coils')
    Modbus15( m, address, values ); % tcpip_pipe, Address, binaryVector
elseif strcmp(target, 'holdingregs')
    %error('only "coils" implemented till now');
    Modbus16( m, address, values );
else
    error('inv target');
end

if debug_level('get')==1
    if length(values)==1
        fprintf(1, 'write target=%s address=%d value=%d\n', target, address, values);
    else
        fprintf(1, 'write target=%s address=%d len(values)=%d\n', target,address,length(values));
    end
end


% ----------------------------------------------------------------------
function tcpip_pipe= modbus_ini0

IPADDR = '127.0.0.1';          % IP Address
PORT = 502;                       % TCP port
tcpip_pipe = tcpip(IPADDR, PORT); %IP and Port 
set(tcpip_pipe, 'InputBufferSize', 512); 
tcpip_pipe.ByteOrder='bigEndian';
global modbus_last_err
try 
    if ~strcmp(tcpip_pipe.Status,'open') 
        fopen(tcpip_pipe); 
    end
    %disp('TCP/IP Open'); 
    modbus_last_err= 0; % NO ERR
catch err 
    %disp('Error: Can''t open TCP/IP'); 
    modbus_last_err= 1;
end


function fbValue= Modbus1( tcpip_pipe, Address, nBits0 )
%
% Read "nBits" starting at "Address"
% e.g. Address==100 and nBits=2 would return a 1x2 array with %M100 and %M101
%
% This function was downloaded from:
% https://www.mathworks.com/matlabcentral/answers/73725-modbus-over-tcp-ip
% zip file given by Jeff, 22 Dec 2016

if nargin<1
    tcpip_pipe= modbus_ini0;
end
if nargin<2
    Address= 100; % start address at %M100
end
if nargin<3
    nBits0= 8;
end
if nBits0>16
    error('nBits0>16')
end

% Read 16 coils -------------------------------------------
transID = uint8(0);                 % initialize transID
transID = uint8(transID+1);         % Transaction Identifier 
ProtID = uint8(0);                  % Protocol ID (0 for ModBus) 
Length = uint8(6);                  % Remaining bytes in message
UnitID = uint8(1);                  % Unit ID (1) makes no difference 
FunCod = uint8(1);                  % Fuction code: read coils(1) 

% Address = 101-1;                    % Start Address 400102 = 101 (1-65536)

AddressHi = uint8(fix(Address/256));% Converts address to 8 bit 
AddressLo = uint8(Address-fix(Address/256)*256);

% Value = uint8(16);                  % number of bits (0-255)
nBits = 16;
Value = uint8(nBits);                  % number of bits (0-255)

ValueHi = uint8(fix(Value/256));    % Converts value to 8 bit
ValueLo = uint8(Value-fix(Value/256)*256);
message = [0; transID; 0; ProtID; 0; Length; UnitID; FunCod; ...
    AddressHi; AddressLo; ValueHi; ValueLo]; 

% write to PLC command
fwrite(tcpip_pipe, message, 'uint8');
while ~tcpip_pipe.BytesAvailable,end

% Read back from PLC command ---------------------------------
readback = fread( tcpip_pipe, tcpip_pipe.BytesAvailable ); %reads response in 8bit integer

%fclose( tcpip_pipe ); % call "mymodbus_m16('end')" to do the fclose

% fbtransID = readback(1)*256+readback(2);
% fbProtID  = readback(3)*256+readback(4);
% fbLength  = readback(5)*256+readback(6);
% fbUnitID  = readback(7);
% fbFunCod  = readback(8);
% fbbytes   = readback(9);
% test1     = readback(10);
% test11    = readback(11);
test      = readback(11)*256 +readback(10);

fbValue = decimalToBinaryVector( test, nBits, 'LSBFirst' );  %contains coils from PLC
fbValue = fbValue(1:nBits0);


function fbValue= Modbus3( tcpip_pipe, Address, nWords )
% Get one array of N words

% % configuration of TCP/IP channel ---------------------
% IPADDR = '127.0.0.1';          % IP Address
% PORT = 502;                       % TCP port
% tcpip_pipe = tcpip(IPADDR, PORT); %IP and Port 
% set(tcpip_pipe, 'InputBufferSize', 512); 
% tcpip_pipe.ByteOrder='bigEndian';
% try 
%     if ~strcmp(tcpip_pipe.Status,'open') 
%         fopen(tcpip_pipe); 
%     end
%     disp('TCP/IP Open'); 
% catch err 
%     disp('Error: Can''t open TCP/IP'); 
% end

% Read multiple 16 bit unsigned integers -------------------------------------------
transID = uint8(0);                 % initialize transID
transID = uint8(transID+1);         % Transaction Identifier 
ProtID = uint8(0);                  % Protocol ID (0 for ModBus) 
Length = uint8(6);                  % Remaining bytes in message
UnitID = uint8(1);                  % Unit ID (1) makes no difference 
FunCod = uint8(3);                  % Fuction code: read registers(3) 

% Address = 101-1;                    % Start Address 400102 = 101 (1-65536)
AddressHi = uint8(fix(Address/256));% Converts address to 8 bit 
AddressLo = uint8(Address-fix(Address/256)*256);

% Value = uint8(10);                  % number of registers (1-65536)
Value = uint8(nWords);
ValueHi = uint8(fix(Value/256));    % Converts value to 8 bit
ValueLo = uint8(Value-fix(Value/256)*256);
message = [0; transID; 0; ProtID; 0; Length; UnitID; FunCod; AddressHi; AddressLo; ValueHi; ValueLo]; 
% write to PLC command
fwrite(tcpip_pipe, message, 'uint8');
while ~tcpip_pipe.BytesAvailable,end

% Read back from PLC command ---------------------------------
readback = fread( tcpip_pipe, tcpip_pipe.BytesAvailable ); %reads response in 8bit integer
% fbtransID = readback(1)*256+readback(2);
% fbProtID = readback(3)*256+readback(4);
% fbLength = readback(5)*256+readback(6);
% fbUnitID = readback(7);
% fbFunCod = readback(8);
fbbytes = readback(9);
for c = 1:2:fbbytes
    fbValue((c+1)/2)= readback(10+c)+256*readback(9+c);  %contains data from PLC
end

% fclose(tcpip_pipe);
% fbValue


function Modbus15( tcpip_pipe, Address, binaryVector )
%
% Write the binaryVector atarting at Address
% e.g. if Address=120 and binaryVector= [0 1 0] then %M120 and %M122 would
% become zero and %M121 would become 1.
%
% This function was downloaded from:
% https://www.mathworks.com/matlabcentral/answers/73725-modbus-over-tcp-ip
% zip file given by Jeff, 22 Dec 2016

if nargin<1
    tcpip_pipe= modbus_ini0;
    Address= 107; %100; %120;
    binaryVector= ones(1,5); %zeros(1,15); %ones(1,16);
end
if length(binaryVector)>10
    split_bits_write( tcpip_pipe, Address, binaryVector );
    return
end

% Write multiple coils -------------------------------------------
transID = uint8(0);              % initialize transID
transID = uint8(transID+1);      % Transaction Identifier 
ProtID = uint8(0);               % Protocol ID (0 for ModBus) 

% SentCoil = uint8(10);            % Number of coils to send 
SentCoil = uint8(length(binaryVector));            % Number of coils to send 

BytesData = uint8(SentCoil/8+1); % Number of bytes of data being sent
Length = uint8((SentCoil/8)+8);  % Remaining bytes in message
UnitID = uint8(1);               % Unit ID (1) makes no difference 
FunCod = uint8(15);              % Fuction code: write muliple registers(16) 

%Address = 101-1;                 % Start Address 400102 = 101 (1-65536)
%Address = 111-1;                 % Start Address 400102 = 111 (1-65536)

AddressHi = uint8(fix(Address/256)); % Converts address to 8 bit 
AddressLo = uint8(Address-fix(Address/256)*256);

% binaryVector = [0 0 1 0 1 0 0 1 1 0];  %Data bits to send
% binaryVector = [0 0 1 0 1 0 0 1 1 0 1 1 1 1 1 1];  %Data bits to send
% binaryVector = ones(1,5);  %Data bits to send % there is a max of 10bits
% binaryVector = [1 1 1 1 1 1];  %Data bits to send

Value = binaryVectorToDecimal(binaryVector,'LSBFirst'); % Converts bit array to decimal
ValueHi = uint8(fix(Value/256)); % Converts value to 8 bit
ValueLo = uint8(Value-fix(Value/256)*256);
message = [0; transID; 0; ProtID; 0; Length; UnitID; FunCod; ...
    AddressHi; AddressLo; 0; SentCoil; BytesData; ValueLo; ValueHi];

% write to PLC command
fwrite(tcpip_pipe, message, 'uint8');
while ~tcpip_pipe.BytesAvailable,end

%fclose(tcpip_pipe); % call "mymodbus_m16('end')" to do the fclose


function split_bits_write( tcpip_pipe, Address0, binaryVector )
Address= Address0;
for ind= 1:10:length(binaryVector)
    ind2= ind+10-1;
    if ind2>length(binaryVector)
        ind2= length(binaryVector);
    end
    Modbus15( tcpip_pipe, Address, binaryVector(ind:ind2) )
    Address= Address+10;
end
return


function Modbus16( tcpip_pipe, Address, wordsVector )
% Put one array of words

% % configuration of TCP/IP channel ---------------------
% IPADDR = '127.0.0.1';          % IP Address
% PORT = 502;                       % TCP port
% tcpip_pipe = tcpip(IPADDR, PORT); % IP and Port 
% set(tcpip_pipe, 'InputBufferSize', 512); 
% tcpip_pipe.ByteOrder='bigEndian';
% try 
%     if ~strcmp(tcpip_pipe.Status,'open') 
%         fopen(tcpip_pipe); 
%     end
%     disp('TCP/IP Open'); 
% catch err 
%     disp('Error: Can''t open TCP/IP'); 
% end

% Write multiple 16 bit unsigned integers ---------------------------------
transID = uint8(0);              % initialize transID
transID = uint8(transID+1);      % Transaction Identifier 
ProtID = uint8(0);               % Protocol ID (0 for ModBus) 

% SentReg = uint8(1);              % Number of registers to send (max 255)
% wordsVector= 2392+(0:4);
SentReg = uint8(length(wordsVector));              % Number of registers to send (max 255)

BytesData = uint8(2*SentReg);    % Number of bytes of data being sent
Length = uint8(7+2*SentReg);     % Remaining bytes in message
UnitID = uint8(1);               % Unit ID (1) makes no difference 
FunCod = uint8(16);              % Fuction code: write muliple registers(16) 

% Address = 100; %102-1;                 % Start Address 400102 = 101 (1-65536)
AddressHi = uint8(fix(Address/256)); % Converts address to 8 bit 
AddressLo = uint8(Address-fix(Address/256)*256);

% Value = [2391];                  % Data value (0-65536)
% % ValueHi = uint8(fix(Value/256)); % Converts value to 8 bit
% % ValueLo = uint8(Value-fix(Value/256)*256);
% % arrayBytes= [ValueHi; ValueLo];
% arrayBytes= values2arrayBytes( Value );
arrayBytes= values2arrayBytes( wordsVector );

message = [0; transID; 0; ProtID; 0; Length; UnitID; FunCod; AddressHi; AddressLo; 0; SentReg; BytesData; arrayBytes];
% write to PLC command
fwrite(tcpip_pipe, message, 'uint8');
while ~tcpip_pipe.BytesAvailable,end

% fclose(tcpip_pipe);


function arrayBytes= values2arrayBytes( Value )
% ValueHi = uint8(fix(Value/256)); % Converts value to 8 bit
% ValueLo = uint8(Value-fix(Value/256)*256);
% arrayBytes= [ValueHi; ValueLo];

ValueHi = uint8(fix(Value(:)/256)); % Converts value to 8 bit
ValueLo = uint8(Value(:)-fix(Value(:)/256)*256);
arrayBytes= [ValueHi'; ValueLo'];
arrayBytes= arrayBytes(:);
