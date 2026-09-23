function varargout = mymodbus2( varargin )
%
% Function wrapper to allow Matlab versions both before and after 2017a
% 
% m= mymodbus2('ini');
% mymodbus2('read', m, target, address, count);
% mymodbus2('write', m, target, address, values);
% mymodbus2('end', m);

% Note: in Matlab versions < 2017a every read/write implies 'end' and 'ini'
% it is a bug / incomplete modbus protocol implementation
% Note2: the Java implementation is used following this same ini/end idea
% because of communication timeouts found in some servers; failing a
% timeout would imply a hard to reconnect future

% Debugging:
% mymodbus2( 'db_level_set', 0 )
%   no debug or stop debug
% mymodbus2( 'db_level_set', 1 )
%   activate printf based debug
% mymodbus2( 'db_level_set', 2 )
%   activate read/write times

% Mar2020, Mar2021 (use Java interface), JG

% develop mymodbus2.m
if nargin<1 && exist('mymodbus2_tst.m', 'file')
    mymodbus2_tst
    return
end

% debugger is run before (has priority over) all operations:
cmd= varargin{1};
if cmd(1)=='d'
    if nargout<1
        debug_level( varargin{:} );
    else
        [varargout{1:nargout}]= debug_level( varargin{:} );
    end
    return
end

% general Modbus commands
global ModbusInterfaceToUse
%   to show: global ModbusInterfaceToUse; ModbusInterfaceToUse
%   to set:  global ModbusInterfaceToUse; ModbusInterfaceToUse=2
if isempty(ModbusInterfaceToUse)
    ModbusInterfaceToUse= select_ModbusInterfaceToUse();
end

switch ModbusInterfaceToUse
    case 0
        % old Matlab versions
        [varargout{1:nargout}]= mymodbus2_m16( varargin{:} );
    case 1
        % recent Matlab versions
        [varargout{1:nargout}]= mymodbus2_m17( varargin{:} );
    case 2
        % old and recent Matlab versions, using a Java library
        [varargout{1:nargout}]= mymodbus2_java( varargin{:} );
    case 3
        % old and recent Matlab versions, using a Java library
        [varargout{1:nargout}]= mymodbus2_tcpclient( varargin{:} );
    otherwise
        error('invalid ModbusInterfaceToUse')
end

return; % end of main function


% ======================================================================
function ModbusInterfaceToUse= select_ModbusInterfaceToUse

% by default, choose a Java Modbus interface
if ~exist('mymodbus2_cnf.m', 'file')
    ModbusInterfaceToUse= 3; %2;
    return
end

% alternatively, decide based on data in a local file
ret= mymodbus2_cnf();
ModbusInterfaceToUse= ret.ModbusInterfaceToUse;
if ModbusInterfaceToUse ~= -1
    return
end

% file indicated ModbusInterfaceToUse==-1
% file indicated does not want to choose, just continue

% alternatively, decide based on Matlab version
if verLessThan('matlab','9.2') %0 %1
    ModbusInterfaceToUse= 0;
else
    ModbusInterfaceToUse= 1;
end

return; % end of function


function mymodbus2_config( op, a1, a2 )
% To chose using Java:
% mymodbus2( 'cnf', 'mk_file', 'ModbusInterfaceToUse', 2 )
% To return to the default:
% mymodbus2( 'cnf', 'mk_file', 'ModbusInterfaceToUse', -1 )

switch op
    case 'mk_file'
        pname= fileparts( which('mymodbus2') );
        fname= 'mymodbus2_cnf.m';
        mymodbus2_config_mk_file( pname, fname, a1, a2 );
        global ModbusInterfaceToUse
        ModbusInterfaceToUse= [];
    otherwise
        error('inv op')
end


function mymodbus2_config_mk_file( pname, fname, a1, a2 )

fname= fullfile( pname, fname );
[~,fn,~]= fileparts(fname);

newline= sprintf('\n');
str= ['function ret= ' fn newline 'ret= struct("' a1 '", ' num2str(a2) ');' newline];
str= strrep(str, '"', '''');

fid= fopen( fname, 'wt' );
fprintf( fid, '%s', str );
fclose(fid);

return


% ======================================================================

function ret= debug_level( op, a1, a2 )
%
% Internal calls: 'ini', 'get', 'set', 'lock'
%   debug_level('get')
%
% Outside calls, all start by letter 'd':
%   ret= mymodbus2( 'db_level', 'get', [] )
%   mymodbus2( 'db_level_set', 0 )   % 0/1/2 )
%   mymodbus2( 'db_level_lock', 0 )  % 0/1 )
%   mymodbus2( 'db_ini_data_log' )
%   mymodbus2( 'db_end_data_log' )

persistent dbLevel
if isempty(dbLevel)
    dbLevel= 0;
end

if nargin<1
    % do a early return for faster performance
    ret= dbLevel; return
end

persistent dbLevelLock
if isempty(dbLevelLock)
    dbLevelLock= 0;
end

% full options of ret= debug_level( ... )
ret= [];
switch op
    case 'ini', if ~dbLevelLock, dbLevel= 0; end
    case 'set', if ~dbLevelLock, dbLevel= a1; end
    case 'get', ret= dbLevel;
    case 'lock', ret= dbLevelLock; dbLevelLock= a1;

    case 'db_level'
        % avoid this general entry point
        % try to use specific commands
        if nargin<3
            ret= debug_level(a1);
        else
            ret= debug_level(a1,a2);
        end

    case 'db_level_set',  debug_level('set', a1);
    case 'db_level_lock', debug_level('lock', a1);

    case 'db_log_reset'
        debug_comms_log_reset();
    case 'db_ini_data_log'
        % protect debug repeated 'ini' to clear data
        clearFlag= 1;
        if nargin>1
            clearFlag= a1;
        end
        if clearFlag
            debug_comms_log_reset();
        end
        debug_level( 'set', 2 );
        debug_level( 'lock', 1 );
    case 'db_end_data_log'
        % end protection on debug repeated 'ini' clearing data
        debug_level( 'lock', 0 );
        debug_level( 'set', 0 );

    case 'db_info'
        % mymodbus2( 'db_info' )
        debug_comms_log_info_show();
        fprintf(1, 'mymodbus2 dbLevel: %d\n', dbLevel);

    otherwise, error('inv op');
end


function debug_comms_show(writeFlag, target, address, ret)
if writeFlag
    str1= 'write'; str2= 'values';
else
    str1= 'read';  str2= 'ret';
end
if length(ret)==1
    fprintf(1, '%s target=%s address=%d %s=%d\n', str1, target, address, str2, ret);
else
    fprintf(1, '%s target=%s address1=%d len(%s)=%d\n', str1, target, address(1), str2, length(ret));
end


function debug_comms_log_info_show()
global DBC
fprintf(1, 'mymodbus2 events logged: %d\n', size(DBC,2));


function debug_comms_log_reset()
global DBC
DBC= [];


function debug_comms_log(writeFlag, t0, target, address, values0)
% how to test this: lauch myterminal4 or 5 and press buttons/keys
% choosing a global var to handle function crash or forgotten plot

global DBC

t= now; % time after modbus command (t0 is the time of sending)
c= target(1)*10 +writeFlag;
a= address(1);
n= length(values0);

% save binary values (strings cannot be fully saved)
if target(1)=='c'
    % binary values
    v= bi2de(values0(:)'>0);
else
    % register values
    values= double(values0(:)');
    L= length(values);
    if L>10+8 % myterminal* usually has strings after 10values
        v= min(values(11:18), 255);
        v= sum( v(:)'.*(256.^(7:-1:0)) );
    elseif L>10 % L<10+8
        v= min(values(11:end), 255);
        v= sum( v(:)'.*(256.^(length(v)-1:-1:0)) );
    elseif L>8 % L=9 or L=10
        v= min(values(end-7:end), 255);
        v= sum( v(:)'.*(256.^(length(v)-1:-1:0)) );
    else % L<=8
        v= min(values, 255); % save just the low words, loose high words
        v= sum( v(:)'.*(256.^(length(v)-1:-1:0)) );
    end
end

% DBC is 5xN, N=#events
% time is in DBC(n,1:2), marking start/end modbus command
% data is all in the 1x1 v value, DBC(n,5)
DBC(:,end+1)= [t0 t c n*1000+a v]';


function debug_comms( writeFlag, t0, target, address, ret )
switch debug_level()
    case 0
        % do nothing
    case 1
        % fprintf debug output
        debug_comms_show( writeFlag, target, address, ret );
    case 2
        % internal log of commands
        debug_comms_log( writeFlag, t0, target, address, ret );
    otherwise
        % should not happen
        warning('inv comms debug level');
end


% ======================================================================
function ret= mymodbus2_tcpclient( cmd, a1, a2, a3, a4 )
% Using Matlab's tcpclient(), should work on all versions

switch cmd
    case 'cnf', mymodbus2_config( a1, a2, a3 );
    case 'ini', debug_level('ini'); ret= modbus_ini3;
    case 'end', modbus_end3(a1);  % a1= ret= modbus_ini3();
    case 'read', ret= myread3( a1, a2, a3, a4 ); % m, target, address, count
    case 'write', mywrite3( a1, a2, a3, a4 ); % m, target, address, values
        %     case 'db_level', ret= debug_level(a1, a2); % ret= mymodbus2( 'db_level', 'get', [] )
        %     case 'db_level_set',  debug_level('set', a1); % mymodbus2( 'db_level_set', 0/1/2 )
        %     case 'db_level_lock', debug_level('lock', a1); % mymodbus2( 'db_level_lock', 0/1 )
    otherwise
        error('inv cmd')
end


function m= modbus_ini3
IPADDR = mymodbus2_opt('ip'); % '127.0.0.1';          % IP Address
PORT = mymodbus2_opt('port'); % 502;                       % TCP port
global modbus_last_err

try 
    m = tcpclient(IPADDR, PORT); %IP and Port
    %set(m, 'InputBufferSize', 512);
    % ^^ set() for buffer size, was used for tcpip
    %    is automatically handled in tcpclient
    m.ByteOrder= 'big-endian';
    %disp('TCP/IP Open'); 
    modbus_last_err= 0; % NO ERR
catch err 
    %disp('Error: Can''t open TCP/IP'); 
    modbus_last_err= 1;
end


function modbus_end3(m)
clear m


function ret= myread3(m, target, address, count)
% error('yet to implement')

% target is 'coils' or 'holdingregs'
% address: 1x1 : memory address 0,1,2,...
t0= now;
if target(1)=='c'
    %ret= m.ReadCoils(address, count);
    ret= Modbus1( m, address, count ); % tcpip_pipe, Address, nBits0
else
    %ret= m.ReadHoldingRegisters(address, count);
    ret= Modbus3( m, address, count );
end
debug_comms(0, t0, target, address, ret );


function mywrite3(m, target, address, values)
% error('yet to implement')

% target is 'coils' or 'holdingregs'
% address: 1x1 : memory address 0,1,2,...
t0= now;
if target(1)=='c'
    %m.WriteMultipleCoils( address, values );
    Modbus15( m, address, values ); % tcpip_pipe, Address, binaryVector
else
    %m.WriteMultipleRegisters( address, values );
    Modbus16( m, address, values );
end
debug_comms(1, t0, target, address, values);



% ======================================================================
function ret= mymodbus2_java( cmd, a1, a2, a3, a4 )
% Modbus for Matlab based on EasyModbus JAR (v2.4)

switch cmd
    case 'cnf', mymodbus2_config( a1, a2, a3 );
    case 'ini', debug_level('ini'); ret= modbus_ini2;
    case 'end', modbus_end2(a1);
    case 'read', ret= myread2( a1, a2, a3, a4 ); % m, target, address, count
    case 'write', mywrite2( a1, a2, a3, a4 ); % m, target, address, values
        %     case 'db_level', ret= debug_level(a1, a2); % ret= mymodbus2( 'db_level', 'get', [] )
        %     case 'db_level_set',  debug_level('set', a1); % mymodbus2( 'db_level_set', 0/1/2 )
        %     case 'db_level_lock', debug_level('lock', a1); % mymodbus2( 'db_level_lock', 0/1 )
    otherwise
        error('inv cmd')
end


function m= modbus_ini2
persistent JAP
if isempty(JAP)
    %javaaddpath('.\EasyModbusJava.jar')
    p= which('EasyModbusJava.jar');
    javaaddpath(p);
    JAP= 1;
end

% next exps work well on Matlab 2016a, 2020a (all 64bits)
% fail on Matlab 2015a (32 bits), likely because of incomplatible java libs
method= 3;
switch method
    case 0
        % ok, this works, but is only a local address...
        import ModbusClient.*;
        m= ModbusClient('127.0.0.1', 502);
    case 1
        % many times this fails (almost always if right after Matlab boot)
        import ModbusClient.*;
        m= ModbusClient('192.168.27.180', 502);
    case 2
        % fails most of the times
        import ModbusClient.*;
        m= ModbusClient('192.168.27.208', 502);
    case 3
        % fails most of the times
        ipaddr= mymodbus2_opt('ip');
        port= mymodbus2_opt('port');
        import ModbusClient.*;
        m= ModbusClient(ipaddr, port);

    case 9
        % this always fails
        ipaddr= mymodbus2_opt('ip'); port= mymodbus2_opt('port');
        if strcmp(ipaddr, '127.0.0.1')
            import ModbusClient.*;
            m= ModbusClient(ipaddr, port);
        else
            for i=1:5
                [m, okFlag, ME]= ModbusClient_try(ipaddr, port);
                if okFlag
                    break;
                end
                fprintf(1, 'i=%d\n', i);
                pause(0.5);
            end
            if ~okFlag
                rethrow(ME)
            end
        end
end
m.Connect();
return


function [m, okFlag, ME]= ModbusClient_try(ipaddr, port)
try
    % fails here, conflit try and import?
    import ModbusClient.*;
    m= ModbusClient(ipaddr, port);
    okFlag= 1;
catch ME
    m= [];
    okFlag= 0;
end


function modbus_end2( m )
m.Disconnect();


function ret= myread2(m, target, address, count)
% target is 'coils' or 'holdingregs'
% address: 1x1 : memory address 0,1,2,...
t0= now;
if target(1)=='c'
    ret= m.ReadCoils(address, count);
else
    ret= m.ReadHoldingRegisters(address, count);
end
debug_comms(0, t0, target, address, ret );


function mywrite2(m, target, address, values)
% target is 'coils' or 'holdingregs'
% address: 1x1 : memory address 0,1,2,...
t0= now;
if target(1)=='c'
    m.WriteMultipleCoils( address, values );
else
    m.WriteMultipleRegisters( address, values );
end
debug_comms(1, t0, target, address, values);



% ======================================================================
function ret= mymodbus2_m17( cmd, a1, a2, a3, a4 )
% Modbus for Matlab versions >= 2017a (i.e. ones that have modbus.m)

switch cmd
    case 'cnf', mymodbus2_config( a1, a2, a3 );
    case 'ini', debug_level('ini'); ret= modbus_ini;
    case 'end' % do nothing
    case 'read', ret= myread( a1, a2, a3, a4 ); % m, target, address, count
    case 'write', mywrite( a1, a2, a3, a4 ); % m, target, address, values
    otherwise
        error('inv cmd')
end


function m= modbus_ini
%m= modbus('tcpip', '127.0.0.1', 502);
m= modbus('tcpip', mymodbus2_opt('ip'), mymodbus2_opt('port') );


function ret= myread(m, target, address, count)
% target is 'coils' or 'holdingregs'
% address: 1x1 : memory address 0,1,2,...
t0= now;
ret= read(m, target, address+1, count); % modbus uses 1,2... instead of 0,1...
debug_comms(0, t0, target, address, ret );


function mywrite(m, target, address, values)
% target is 'coils' or 'holdingregs'
% address: 1x1 : memory address 0,1,2,...
t0= now;
write(m,target,address+1,values); % modbus uses 1,2... instead of 0,1...
debug_comms(1, t0, target, address, values);


% ======================================================================
function ret= mymodbus2_m16( cmd, a1, a2, a3, a4 )
%
% Modbus use with Matlab versions before 2017a

% Mar2020, JG

switch cmd
    case 'cnf', mymodbus2_config( a1, a2, a3 );
    case 'ini', debug_level('ini'); ret= modbus_ini0; % ret == m
    case 'end', fclose( a1 ); % a1 = m
    case 'read', ret= myread0( a1, a2, a3, a4 ); % m, target, address, count
    case 'write', mywrite0( a1, a2, a3, a4 ); % m, target, address, values
    otherwise
        error('inv cmd')
end


% ----------------------------------------------------------------------
function tcpip_pipe= modbus_ini0

IPADDR = mymodbus2_opt('ip'); % '127.0.0.1';          % IP Address
PORT = mymodbus2_opt('port'); % 502;                       % TCP port
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


function ret= myread0(m, target, address, count)
%ret= read(m, target, address+1, count); % modbus uses 1,2... instead of 0,1...

t0= now;

if strcmp(target, 'coils')
    ret= Modbus1( m, address, count ); % tcpip_pipe, Address, nBits0
elseif strcmp(target, 'holdingregs')
    %error('only "coils" implemented till now');
    ret= Modbus3( m, address, count );
else
    error('inv target');
end

debug_comms(0, t0, target, address, ret);


function mywrite0(m, target, address, values)
%write(m,target,address+1,values); % modbus uses 1,2... instead of 0,1...

t0= now;

if strcmp(target, 'coils')
    Modbus15( m, address, values ); % tcpip_pipe, Address, binaryVector
elseif strcmp(target, 'holdingregs')
    %error('only "coils" implemented till now');
    Modbus16( m, address, values );
else
    error('inv target');
end

debug_comms(1, t0, target, address, values);


% ----------------------------------------------------------------------
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
    error('nBits0>16')      % is a problem just 16bits?
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

if isa(tcpip_pipe, 'tcpclient')
    % using tcpclient
    %error('yet to implement')
    % write to PLC command
    %write(tcpip_pipe, message, 'uint8');
    %while ~tcpip_pipe.NumBytesAvailable,end
    write(tcpip_pipe, uint8(message) );
    while ~tcpip_pipe.BytesAvailable, end
    
    % Read back from PLC command ---------------------------------
    %readback = read( tcpip_pipe, tcpip_pipe.NumBytesAvailable ); %reads response in 8bit integer
    readback = read( tcpip_pipe, tcpip_pipe.BytesAvailable ); %reads response in 8bit integer
else
    % isa(tcpip_pipe, 'tcpip') == true
    % write to PLC command
    fwrite(tcpip_pipe, message, 'uint8');
    while ~tcpip_pipe.BytesAvailable,end
    
    % Read back from PLC command ---------------------------------
    readback = fread( tcpip_pipe, tcpip_pipe.BytesAvailable ); %reads response in 8bit integer
end

%fclose( tcpip_pipe ); % call "mymodbus2_m16('end')" to do the fclose

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

if isa(tcpip_pipe, 'tcpclient')
    % using tcpclient
    %error('yet to implement')
    % write to PLC command
    %write(tcpip_pipe, message, 'uint8');
    %while ~tcpip_pipe.NumBytesAvailable, end
    write(tcpip_pipe, uint8(message));
    while ~tcpip_pipe.BytesAvailable, end
    
    % Read back from PLC command ---------------------------------
    %readback = read( tcpip_pipe, tcpip_pipe.NumBytesAvailable ); %reads response in 8bit integer
    readback = read( tcpip_pipe, tcpip_pipe.BytesAvailable ); %reads response in 8bit integer
else
    % isa(tcpip_pipe, 'tcpip') == true
    % write to PLC command
    fwrite(tcpip_pipe, message, 'uint8');
    while ~tcpip_pipe.BytesAvailable, end
    
    % Read back from PLC command ---------------------------------
    readback = fread( tcpip_pipe, tcpip_pipe.BytesAvailable ); %reads response in 8bit integer
end

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
% Write the binaryVector starting at Address
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
    % call multiple times Modbus15
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
if isa(tcpip_pipe, 'tcpclient')
    % using tcpclient
    %error('yet to implement')
    %write(tcpip_pipe, message, 'uint8');
    %while ~tcpip_pipe.NumBytesAvailable, end
    write(tcpip_pipe, uint8(message));
else
    % isa(tcpip_pipe, 'tcpip') == true
    fwrite(tcpip_pipe, message, 'uint8');
    while ~tcpip_pipe.BytesAvailable, end
end

%fclose(tcpip_pipe); % call "mymodbus2_m16('end')" to do the fclose


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
if isa(tcpip_pipe, 'tcpclient')
    % using tcpclient
    %error('yet to implement')
    %write(tcpip_pipe, message, 'uint8');
    %while ~tcpip_pipe.NumBytesAvailable, end
    write(tcpip_pipe, uint8(message));
else
    % isa(tcpip_pipe, 'tcpip') == true
    % write to PLC command
    fwrite(tcpip_pipe, message, 'uint8');
    while ~tcpip_pipe.BytesAvailable,end
end

% fclose(tcpip_pipe);


function arrayBytes= values2arrayBytes( Value )
% ValueHi = uint8(fix(Value/256)); % Converts value to 8 bit
% ValueLo = uint8(Value-fix(Value/256)*256);
% arrayBytes= [ValueHi; ValueLo];

ValueHi = uint8(fix(Value(:)/256)); % Converts value to 8 bit
ValueLo = uint8(Value(:)-fix(Value(:)/256)*256);
arrayBytes= [ValueHi'; ValueLo'];
arrayBytes= arrayBytes(:);
