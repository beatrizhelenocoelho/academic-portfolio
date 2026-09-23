function [ret, ret2]= myterminal5_aux( cmd, a1, a2, a3, a4 )
%
% This function exists to take code out of function myterminal5.m
%
% Typical usage: this function is called from myterminal5.m
%
% Other usages (examples):
% myterminal5_aux( 'set_coils', 0, zeros(1:5) )
% myterminal5_aux( 'get_coils', 0, 10 )

% Mar2020, Apr2021, JG

if nargin<1
    myterminal5;
    return
end

ret= [];
switch cmd
    case 'get_coils'
        ret= get_coils( a1, a2 ); % ( firstCoil, numCoils );
    case 'get_regs'
        ret= get_regs( a1, a2 );
    case 'get_coils_and_regs'
        [ret, ret2]= get_coils_and_regs( a1, a2, a3, a4 );

    case 'set_regs'
        set_regs( a1, a2 );
    case 'set_coils'
        set_coils( a1, a2); % ( coilNum, value(s) );
    case 'set_coils_and_regs'
        set_coils_and_regs( a1, a2, a3, a4 );
        
    case 'key_push_plc_gui'
        % KeyId push to PLC and GUI
        key_push_plc_gui( a1, a2, a3); % ( keyId, resetFlag, handle )
    case 'key_push'
        keyshandler( a1 );
        
    case 'refresh'
        % call a "while 1" loop in myterminal_refresh()
        if nargin>= 3
            % call from the fig callback
            %myterminal_refresh( a1, a2 ); % ( hObject=a1, handles=a2 )
            myterminal_refresh_cmd( 'fig_refresh', a1, a2 );
        else
            % command line call, refresh during 5sec
            myterminal_refresh_cmd( 'fig_refresh_timed', [], [], 5 );
        end
    case {'tu_run', 'tu_keys_run'}
        % run timed list of coils (set coils along time) or
        % run timed list of pressed keys (instead of directly setting coils)
        %
        % myterminal5_aux( 'tu_run', mfname )
        % myterminal5_aux( 'tu_run', mfname, t1t2 )
        cmd2= [cmd '_mfname']; % e.g. tu_run_mfname
        if nargin>=3
            % direct command line call
            myterminal_refresh_cmd( cmd2, [], [], a2, a1 );
        elseif nargin>=2
            % direct command line call
            myterminal_refresh_cmd( cmd2, [], [], [], a1 );
        elseif nargin==1
            % direct command line call, but ask mfname
            myterminal_refresh_cmd( cmd2, [], [], [], [] );
        end

    case 'addr_cnf'
        % get offset addresses
        % usage: ret= myterminal5_aux('addr_cnf',0,[])
        ret= plc_comm_addresses_config( a1, a2 );
    case 'addr_cnf_show'
        % usage: myterminal5_aux('addr_cnf_show');
        plc_comm_addresses_config_show();
        
    case 'options'
        % ret= myterminal_options( op, a1, a2 )
        ret= myterminal_options( a1, a2, a3 );
    case 'log_strings'
        % ret= myterminal5_aux('log_strings', 'get', [])
        % ret= log_strings( op, str )
        ret= log_strings( a1, a2 );

    case 'mymenu'
        % GUI to select options by their names
        mymenu
    case 'mymenu2'
        % command line to select options by their names
        mymenu( a1 )
        
    otherwise
        error('inv cmd');
end


% ----------------------------------------------------------------------
function ret= plc_comm_addresses_config( op, a1 )
% get offset addresses  ADR= plc_comm_addresses_config(0);
% zero offset addresses myterminal5_aux('addr_cnf',-1,[])
% zero offset addresses myterminal5_aux('addr_cnf',1,[0 10 10 0 20])
% higher def addresses  myterminal5_aux('addr_cnf',1,[200 10 10 200 20])

% Address translation options defined in the first version of myterminal
% ADR_default= [180 10 10 180 20]; %M180..189, %M190..199, %MW180..199
% ADR_default= [180 10 10 180 70]; %M180..189, %M190..199, %MW180..249
% ADR_default= [180 10 10 180 70]; %M180..189, %M190..199, %MW180..249
% ADR_default= [0 10 10 20 70]; %m0..9, %m10..19, %mw20..89
% ADR_default= [180 10 10 180 70]; %M180..189, %M190..199, %MW180..249
ADR_default= [0 10 10 180 70]; %M0..9, %M10..19, %MW180..249

% Save options across multiple calls
persistent ADR
if isempty(ADR)
    ADR= ADR_default;
end

ret= [];
switch op
    case 0, ret= ADR; % return current adresses translation
    case 1, ADR= a1;  % set addresses translation, a1 is 1x5
    case 2, ret= ADR(a1); % get one specific field

    case -1 
        % ADR= [0 10 10 0 20]; %M0..9, %M10..19, %MW0..19
        ADR= [0 10 10 0 70]; %M0..9, %M10..19, %MW0..69
        % ADR= [0 10 10 180 70]; %M0..9, %M10..19, %MW180..169

    case 100, ADR= ADR_default;
    case 101, ADR= [180 10 10 180 70];
    case 102, ADR= [0 10 10 180 70];
    case 103, ADR= [0 10 10 0 70];
    case 104, ADR= [0 10 10 20 70];

    otherwise, error('inv op');
end


function str= sprintf_range( s1, s2, i1, i2 )
str= sprintf('%s%%%s%d..%%%s%d', s1,s2,i1,s2,i2);


function plc_comm_addresses_config_show
% usage: myterminal5_aux('addr_cnf_show');

ret= plc_comm_addresses_config( 0, [] ); % get 5 values
a= [ret(1) ret(1)+ret(2)-1];
b= [a(2)+1 a(2)+1+ret(3)-1];
c= [ret(4) ret(4)+10-1];
d= [c(2)+1 c(2)+ret(5)-10-1];

x= {};
x{end+1}= sprintf_range('PLC inputs : ','m', a(1),a(2));
x{end+1}= sprintf_range('PLC outputs: ','m', b(1),b(2));
x{end+1}= sprintf_range('PLC words  : ','mw', c(1),c(2));
x{end+1}= sprintf_range('PLC text   : ','mw', d(1),d(2));
x{end+1}= sprintf('PLC ip addr: %s', mymodbus2_opt('ip') );
x{end+1}= sprintf('[Info retrieved %s]', datetime );
for i=1:length(x)
    fprintf(1, '%s\n', x{i});
end

x{end+1}= '[See this info printed in the terminal]';
msgbox(x);


function addr= plc_comm_addresses

% Define here the PLC memory usage for communications.
% This example uses high addresses in order to decrease the chance of
% conflicts with the programs runing in the PLC.

% The idea is to use near zero addresses everywhere but in the base access
% functions. So the user believes is using low addresses but in practice is
% using high addresses as the low addresses are offset at base comm functions.

% User lower addresses to work for TSX P57 1634M 02.00:
% addr= struct( ...
%     'inpCoilsFirst',  180, ... % first input coil at %M180
%     'inpCoilsNum',     10, ...
%     'outpCoilsFirst', 190, ... % first output coil at %M190
%     'outpCoilsNum',    10, ...
%     'regsFirst',      180, ... % first IO reg at %MW180
%     'regsNum',         20 );

ADR= plc_comm_addresses_config(0);
addr= struct( ...
    'inpCoilsFirst',  ADR(1), ... % first input coil
    'inpCoilsNum',    ADR(2), ...
    'outpCoilsFirst', sum(ADR(1:2)), ... % first output coil
    'outpCoilsNum',   ADR(3), ...
    'regsFirst',      ADR(4), ... % first IO reg
    'regsNum',        ADR(5));

% scan cycle first and last programs:
% for i=0:9, fprintf(1,'%%m%d:=%%m%d;\n', i, i+180); end
% for i=0:9, fprintf(1,'%%m%d:=%%m%d;\n', i+190, i+10); end


function flag= addr_ok( first, maxNum, first2, maxNum2 )
flag= 0;
a1= first; a2= first+maxNum-1;
b1= a1+first2; b2= a1+first2+maxNum2-1;
% test a1 <= b1, b2 <= a2
if a1<=b1 && b1<=a2 && a1<=b2 && b2<=a2
    flag= 1;
end


function [coilValues, regValues]= get_coils_and_regs( firstCoil, nCoils, firstReg, nRegs )
addr= plc_comm_addresses;
m= mymodbus2('ini');

coilValues= [];
if ~isempty(nCoils) && nCoils>0
    %coilValues= myread(m, 'coils', firstCoil, nCoils);
    if ~addr_ok( addr.outpCoilsFirst, addr.outpCoilsNum, firstCoil, nCoils)
       warning('outp coil(s) addr out of range')
    end
    coilValues= mymodbus2('read', m, 'coils', firstCoil +addr.outpCoilsFirst, nCoils);
end

regValues= [];
if ~isempty(nRegs) && nRegs>0
    %regValues= myread(m, 'holdingregs', firstReg, nRegs );
    if ~addr_ok( addr.regsFirst, addr.regsNum, firstReg, nRegs)
       warning('reg(s) addr out of range')
    end
    regValues= mymodbus2('read', m, 'holdingregs', firstReg +addr.regsFirst, nRegs);
end

mymodbus2('end', m);


function set_coils_and_regs( firstCoil, coilValues, firstReg, regValues )
addr= plc_comm_addresses;
m= mymodbus2('ini');

if ~isempty(coilValues)
    %mywrite( m, 'coils', firstCoil, coilValues );
    if ~addr_ok( addr.inpCoilsFirst, addr.inpCoilsNum, firstCoil, length(coilValues))
       warning('inp coil(s) addr out or range')
    end
    firstCoil2= firstCoil +addr.inpCoilsFirst;
    mymodbus2('write', m, 'coils', firstCoil2, coilValues );
end

if ~isempty(regValues)
    %mywrite( m, 'holdingregs', firstReg, regValues );
    if ~addr_ok( addr.regsFirst, addr.regsNum, firstReg, length(regValues))
       warning('reg(s) addr out or range')
    end
    mymodbus2('write', m, 'holdingregs', firstReg +addr.regFirst, regValues );
end

mymodbus2('end', m);


% ----------------------------------------------------------------------
function ret= get_coils( firstCoil, numCoils )
% m = modbus('tcpip', '127.0.0.1', 502);
% %read_outputs = myread(m,'coils',301,5);
% read_outputs = myread(m, 'coils', firstCoil, numCoils);
ret= get_coils_and_regs( firstCoil, numCoils, [], [] );


function set_coils( coilNum, values )
% m = modbus('tcpip', '127.0.0.1', 502);
% mywrite(m,'coils', coilNum, value); %mywrite at address %coilNum in Unity Pro
set_coils_and_regs( coilNum, values, [], [] );


function ret= get_regs( firstReg, nRegs )
[~, ret]= get_coils_and_regs( [], [], firstReg, nRegs );


function set_regs( firstReg, regValues )
set_coils_and_regs( [], [], firstReg, regValues );


% ----------------------------------------------------------------------
function push_key( keyId, resetFlag )
% save data into registers starting at 400
% entry index is registered at 499
if nargin<2
    resetFlag= 0;
end

% write where to write now
buffInd1= 1;
buffInd2= buffInd1+1;
ind= get_regs(buffInd1, 1); %Read at address %MW[1 +offset]
set_regs( buffInd2+ind, keyId );

% define where to write next
ind= ind+1;
if resetFlag
    ind= 0;
end
addr= plc_comm_addresses;
if ind > addr.regsNum-buffInd2-1
    % -buffInd2-1 = -3
    % buffer[0] is another variable
    % buffer[1] is the ind
    % the real buffer starts at buffer[2]
    % enforce not going over end of comms buffer
    ind= addr.regsNum-buffInd2-1;
end
set_regs( buffInd1, ind );


% ----------------------------------------------------------------------
function gui_strcat( handle, str )
% accumulate string
s1= [get(handle, 'String') str];
if length(s1)>10
    s1= s1(end-9:end);
end
% display the string
set(handle, 'String', s1);


function key_push_plc_gui( keyId, resetFlag, handle )
% put the key into the PLC and report it in the terminal
push_key( keyId, resetFlag );

% display the key on the screen
if keyId==10
    str= '*';
elseif keyId==11
    str= '#';
else
    str= num2str(keyId);
end
gui_strcat( handle, str );


% ----------------------------------------------------------------------
function ret= myterminal_options( op, a1, a2 )

persistent MTO
if isempty(MTO)
    MTO= struct('refreshDebug',0, 'refreshPeriod', 0.1, ...
        'keyPressedDuration',1, 'logStrings',0, 'logStringsDebug',0, ...
        'timeout', inf, 'regsNum',30, ...
        'tuInfo', [], 'accKeys', 0, 'showNumberOfKeys', 0 );
    %    'tuInpFname','', 'tuRun',[], 'tu',[], 'tuLineDone',[] );
    % handle events table, tu: col1= time [sec]
    % write coils as needed, see file myterminal5_tu.m
    % tuRun= [0 10], tuStart= tic; if toc(tuStart)>tuRun(2), ... clear all
    % tuRange= now+seconds(tuRun), when to load the table?
    % tuValues= ...
    % if now > tuRange(2), tuRange=[], tuValues=[]
    % when to send data? if new tuInd
end
if MTO.regsNum > plc_comm_addresses_config(2,5)
    % check max regsNum:
    MTO.regsNum= plc_comm_addresses_config(2,5);
end

ret= [];
switch op
    case 'showAll', disp(MTO)
        % usage: myterminal5_aux('options', 'showAll',[],[])

    case 'getAll', ret= MTO;
    case 'get'
        if isfield( MTO, a1 )
            ret= getfield( MTO, a1 );
        else
            warning('field not found %s', a1)
            ret= [];
        end
        
    case 'set'
        MTO= setfield(MTO, a1, a2);

    case 'setRefreshDebug'
        % myterminal_options( 'setRefreshDebug', 1 )
        % myterminal_options( 'setRefreshDebug', 0 )
        myterminal_options( 'set', 'refreshDebug', a1 );

    case 'setRefreshPeriod'
        % myterminal_options( 'setRefreshPeriod', 1 )
        % myterminal_options( 'setRefreshPeriod', 0.1 )
        myterminal_options( 'set', 'refreshPeriod', a1 );
        
    case 'setKeyPressedDuration'
        myterminal_options( 'set', 'keyPressedDuration', a1 );

    case 'setLogStrings'
        myterminal_options( 'set', 'logStrings', a1 );
        
    otherwise
        warning('inv op %s', op)
end


function ret= log_strings( op, str )
persistent LS
persistent LS_last_str
if isempty(LS)
    LS= {};
end

ret= LS;

switch op
    case 'get'
        % do nothing, just return LS
    case 'show'
        if isempty(LS)
            msgbox('** no strings logged till now **', 'Logged strings')
        else
            msgbox(LS, 'Logged strings')
        end
        
    case 'reset'
        LS= {};
        log_strings( 'show' );

    case 'push'
        % command line show events (debug)
        if ~strcmp(str, LS_last_str) && ...
                myterminal_options( 'get', 'logStringsDebug' )
            fprintf(1, 'new string: %s\n', str);
            LS_last_str= str;
        end
        
        % if current string equals the previously saved, just do nothing
        if length(LS)>0 && strcmp( str, LS{end} )
            return
        end
        
        % there is some novelty, str may need to be saved in the list
        switch myterminal_options( 'get', 'logStrings' )
            case 0
                % do nothing
            case 1
                % push every novel "tmp" str
                LS{end+1,1}= str;
            case 2
                % just keep final strings (overwrite the incomplete ones)
                if isempty(LS)
                    LS{1,1}= str;
                elseif isempty(LS{end,1}) || strncmp( str, LS{end,1}, length(LS{end,1}) )
                    LS{end,1}= str;
                else
                    LS{end+1,1}= str;
                end
        end
        
    otherwise
        error('inv op');
end


function myterminal_refresh_cmd( op, hObject, handles, dt, mfname  )
%
% This is a preparation function for the loop function myterminal_refresh()

switch op
    case 'fig_refresh'
        % myterminal_refresh_cmd( 'fig_refresh', a1, a2 )
        myterminal_refresh( hObject, handles );

    case 'fig_refresh_timed'
        % myterminal_refresh_cmd( 'fig_refresh_timed', [], [], 5 )
        h= myterminal_options( 'get', 'handles', [] );
        if ~isfield(h, 'REFRESH')
            warning('lost terminal fig data, please reopen the fig');
            return
        end
        %myterminal_options( 'set', 'timeout', datenum(now+seconds(5)) );
        myterminal_options( 'set', 'timeout', datenum(now+seconds(dt)) );
        myterminal_refresh( h.REFRESH, h );
        myterminal_options( 'set', 'timeout', inf);
        
    case 'tu_run'
        % call from the GUI interface (use uigetfile('*.m') ?)
        % set options relative to tu_run
        % time_signals('ini', 'myterminal5_tu');

    case {'tu_run_mfname', 'tu_keys_run_mfname'}
        h= myterminal_options( 'get', 'handles', [] );
        if ~isfield(h, 'REFRESH')
            warning('lost terminal fig data, please reopen the fig');
            return
        end
        
        if ~isempty(mfname)
            % mfname can be filename or tu table
            time_signals('ini', mfname, dt );
        elseif exist('myterminal5_tu.m', 'file')
            % define tu be runing the .m
            time_signals('ini', 'myterminal5_tu.m', dt );
        else
            % ask user the name of the .m to run
            time_signals('ini_gui');
        end
        if strcmp(op, 'tu_keys_run_mfname')
            time_signals('setKeysFlag', 1);
        end
        
        %myterminal_options( 'set', 'timeout', datenum(now+seconds(dt)) );
        %datestr(time_signals('get_t2'))
        myterminal_options( 'set', 'timeout', time_signals('get_t2') );
        myterminal_refresh( h.REFRESH, h );
        myterminal_options( 'set', 'timeout', inf);

end


function myterminal_refresh( hObject, handles )
% Main loop, while(1), doing the refresh of the terminal window

global hObjectSav cnt
hObjectSav= hObject;
if isempty(cnt)
    cnt=1;
else
    cnt= cnt+1;
end

% hObject =
%               Style: 'pushbutton'
%              String: 'Refresh'
%     BackgroundColor: [1 1 1]
%            Callback: [function_handle]
%               Value: 1
%            Position: [5.4444 2.6087 24.4444 3]
%               Units: 'characters'

timeout= myterminal_options('get', 'timeout'); % inf
if ~isinf(timeout) || get(hObject, 'Value') %rem(cnt,2)~=0
    set( hObject, 'String', 'Running' );
    set( hObject, 'ForegroundColor', [0 0 1]);
end

% run "while 1" till a timeout (if using this fn in the command line)
%   or till the user presses the button "Running" (thus getting "Paused")
%
refreshPeriod= myterminal_options('get', 'refreshPeriod');
debugFlag= myterminal_options('get', 'refreshDebug'); %0; %1;
regsNum= myterminal_options('get', 'regsNum');
loopNum= 0;

loopStartTime= now;
while (1)
    loopNum= loopNum+1;
    if debugFlag
        fprintf(1, 'refresh cnt=%d loopNum=%d\n', cnt, loopNum);
    end
    if (now >= timeout) || (isinf(timeout) && ~get(hObject, 'Value'))
        %rem(cnt,2)==0
        if ~isinf(timeout) %, datevec(now), datevec(timeout), 
            fprintf(1, 'Exp. start= %s\n', ...
                datestr(loopStartTime) );
            fprintf(1, 'Exp. end  = %s,  now= %s\n', ...
                datestr(timeout), datestr(now) );
        end
        set( hObject, 'String', 'Paused' );
        set( hObject, 'ForegroundColor', [0 0 0]);
        return
    end
    
    % -- Get PLC output bits:
    
    %[read_outputs, read_mode] = myterminal5_aux( 'get_coils_and_regs', 301,5, 1,1 );
    %[read_outputs, read_mode] = myterminal5_aux( 'get_coils_and_regs', 0,4, 0,1 );
    read_outputs = myterminal5_aux( 'get_coils', 0,4 );
    
    %set( handles.BUZZER,        'Value', read_outputs(1) );
    %set( handles.RED_LED,       'Value', read_outputs(2) );
    %set( handles.YELLOW_LED,    'Value', read_outputs(3) );
    %set( handles.GREEN_LED,     'Value', read_outputs(4) );
    myset( handles.BUZZER,        read_outputs(1), [1 0  0] );
    myset( handles.RED_LED,       read_outputs(2), [1 0  0] );
    myset( handles.YELLOW_LED,    read_outputs(3), [1 .5 0] );
    myset( handles.GREEN_LED,     read_outputs(4), [0 .5 0] );

    % -- Get PLC words (including one string):
    
    %read_mode = myterminal5_aux( 'get_regs', 0,30 );
    read_mode = myterminal5_aux( 'get_regs', 0, regsNum );
    if exist('myterminal5_txt.m', 'file')
        str= myterminal5_txt( read_mode(1) );
    else
        str= sprintf('Mode %d', read_mode(1) );
    end
    set( handles.edit9, 'String', str );
    
    strFromThePLC= message2string( read_mode );
    set( handles.text2display, 'String', strFromThePLC );
    log_strings( 'push', strFromThePLC );
    
    % -- Set PLC input bits (if there is a tu table loaded in memory):

    if time_signals('isactive') % works also if no loaded tu table
        if ~time_signals('getKeysFlag')
            % do directly inputs to the PLC, i.e. direct coils writing
            x= time_signals('getValues');
            set_coils( 0, x );
        else
            % do indirect inputs to the PLC by "auto pressing keys"
            % get new keys and their durations, use them to make inputs
            newKeys= time_signals('getNewKeys');
            %keyshandler( newKeys, 1 ); % 1= singleLoopFlag
            keyshandler( newKeys, 0 ); % NO singleLoopFlag == while(1)
        end
    end
    
    %pause(0.1)
    pause(refreshPeriod)
end


function myset( hObject, value, color )
%set( handles.BUZZER,        'Value', read_outputs(1) );
set( hObject, 'Value', value );
if value
    set( hObject, 'ForegroundColor', color);
else
    set( hObject, 'ForegroundColor', [0 0 0]);
end


function strFromThePLC= message2string( read_mode )
% ignore first 10 values of read_mode
% find a string cropping at the first zero
x= read_mode(11:end);
x= [rem(x(:),256) round(x(:)/256)]';
x= x(:);
ind= find(x==0); ind= [ind; length(x)];
x= x(1:ind(1))';
strFromThePLC= char(x);


% ----------------------------------------------------------------------
function keyshandler( newKey, singleLoopFlag )
%
% simulate the keyboard:
% 1 2 3
% 4 5 6
% 7 8 9
% * 0 #
%
% features:
% each key is pressed for a while
% can consider simultaneous keys
% has a buffer indicating keys are still active
% active keys that go off, imply key-up events
% special keys * and # are represented as numbers 10 and 11

% future work:
% allow pushing a set of keys at multiple time stamps
% simply by allowing newKey to be a matrix tu (with key numbers)
% problem: this is not running in parallel with refresh, so there are no
% PLC outputs recorded by sent / receive messages

% future work:
% represent switches on %m0, %m1, %m2 (using keys 12, 13, 14?)
% problem: these new keys are latched (not pulsed)
% new key presses toggle states?
% better idea: add keys 15, 16, 17 doing key-ups (after 12-14 do key downs)
% which prevails: key-down 12-14 or key-up 15-17?

persistent KH
persistent KHinfo

if nargin<2
    %singleLoopFlag= 0;
    singleLoopFlag= myterminal_options('get', 'accKeys'); % accept keys accumulating
    % -- outside usage / config:
    % myterminal5_aux( 'options', 'set', 'accKeys', 1 )
    % myterminal5_aux( 'options', 'set', 'accKeys', 0 )
    % myterminal5_aux( 'options', 'showAll', [],[] )
end

if isempty(KH)
    KH= {};
    %KH= {11, datenum(now+seconds(0.07))};
end

% save in a list (i) current key pressed and (ii) its death time, i.e. key-up simulation
%
if numel(newKey)==1
    % add one key, time it using a default (options) duration
    KH{end+1,1}= newKey;
    % KH{end,2}= datenum(now+seconds(0.1));
    % KH{end,2}= datenum(now+seconds(1));
    keyPressedDuration= myterminal_options('get', 'keyPressedDuration');
    KH{end,2}= datenum( now +seconds(keyPressedDuration) );
else
    % add zero, one or more timed keys; times need to come along
    % newKey= [k1 dt1; k2 dt2; ... ; kn dtn]'
    % newKey can be empty
    kh2= reshape( newKey, 2, [] );
    for i=1:size(kh2,2)
        KH{end+1,1}= kh2(1,i); % key name (number)
        KH{end,  2}= kh2(2,i); % pressed duration
    end
end
if myterminal_options('get', 'showNumberOfKeys')
    % -- inside usage:
    % myterminal_options( 'set', 'showNumberOfKeys', 1 )
    % myterminal_options( 'set', 'showNumberOfKeys', 0 )
    % -- outside usage:
    % myterminal5_aux( 'options', 'set', 'showNumberOfKeys', 1 )
    % myterminal5_aux( 'options', 'set', 'showNumberOfKeys', 0 )
    fprintf(1, '#keys=%d\n', size(KH,1));
    
    % Problem to solve in the future: Timed out key may still exist at this
    % line of code (see in the next lines the code that removes the timed
    % out key from the buffer). A nice solution would be to report within
    % the "while 1" loop only the changes on the number of keys buffered.
end

% loop get columns using modbus
%
while 1
    % remove timed-out keys from the buffer
    ind= [];
    tnow= now;
    for i= 1:size(KH,1)
        if KH{i,2} < tnow
            ind(end+1)= i;
        end
    end
    KH(ind,:)= [];
    if isempty(KH)
        % stop loop after no keys in buffer
        myterminal5_aux('set_coils', 4,[0 0 0 0]);
        KHinfo= [];
        break
    end
    
    % loop get columns using modbus
    cols= myterminal5_aux('get_coils',4,3);
    lins= calc_lines( cell2mat({KH{:,1}}), cols );

    % if columns or lines did change then save cols and send lines
    if isempty(KHinfo) || max(abs(cols-KHinfo.cols)) || ...
            ~isfield(KHinfo, 'lins') || max(abs(lins-KHinfo.lins))
        KHinfo.cols= cols;
        if ~isfield(KHinfo, 'lins') || max(abs(lins-KHinfo.lins))
            myterminal5_aux('set_coils', 4,lins);
            KHinfo.lins= lins;
        end
    end

    % break in case of single loop
    if singleLoopFlag
        break
    end
end

return


function lins= calc_lines( keyList, cols )
% keyList: array of values in 0:11

% mark all active keys in a matrix 4x3
R= [1 2 3; 4 5 6; 7 8 9; 10 0 11];
M= zeros(4,3);
for k=1:length(keyList)
    [i,j]= find( keyList(k) == R );
    M(i,j)= 1;
end

% unmark unselected (unpowered) columns
% cols= [1 1 1]; % means all columns powered
for k= 1:length(cols)
    if ~cols(k)
        M(:,k)= 0;
    end
end

% return lines info
% lins= [0 0 0 0]; % if no keys pressed
lins= max(M,[],2)';
return


% ----------------------------------------------------------------------
function [ret, ret2]= time_signals( op, a1, a2 )
%
% Read a table tu from file, convert it to current times (now)
% and serve the values as requested

% 5.12.2020 JG

if nargin<1 && exist('time_signals_tst.m', 'file')
    time_signals_tst
    return
end

persistent TS
if isempty(TS)
    TS= TS_ini();
end

ret= [];
ret2= [];
switch op
    case 'ini'
        % time_signals( 'ini', fname, t1t2 )
        TS= TS_ini();
        if nargin>=3
            TS= TS_load( TS, a1, a2 );
        else
            TS= TS_load( TS, a1, [] );
        end
    case 'ini_gui'
        % to use from a GUI
        TS= TS_ini();
        [f,p]= uigetfile('*.m');
        if isequal(f,0)
            return; % user cancel
        end
        TS= TS_load( TS, [p f], [] );
        
    case 'isactive'
        % "now" is in [t1 t2]
        ret= 0;
        if ~isempty(TS.t1t2)
            ret= ( min(TS.t1t2)<=now && now<=max(TS.t1t2) );
        end

    case 'getValues'
        % get all input values at time "now"
        %[ret, tnow, tuInd]= TS_get_values( TS );
        [ret, ret2, tuInd]= TS_get_values( TS );
        TS.tuInd= tuInd;
        
    case 'valuesAreNew'
        % length(tuInd) = 0 at start, 1 at 1st iter, 2 for the rest of time
        if length(TS.tuInd)==1
            ret= 1;
        else
            ret= 0;
            if TS.tuInd(end)~=TS.tuInd(end-1)
                ret= 1;
            end
        end
        
    case 'setKeysFlag', TS.isKeys= a1;
    case 'getKeysFlag', ret= TS.isKeys;

    case 'getNewKeys'
        % get all input values at time "now"
        % ret2 = tnow
        [ret, ret2, tuInd]= TS_get_keys_and_timeouts( TS );
        TS.tuInd= tuInd;
        
    case 'getTable', ret= TS.tu;
    case 'get_t0',   ret= TS.t0;
    case {'get_t1', 'get_tini'}, ret= TS.t1t2(1);
    case {'get_t2', 'get_tend'}, ret= TS.t1t2(2);

    case 'get_t1t2_delta_sec'
        ret= datevec( TS.t1t2 - TS.t0 );
        ret= ret(:,6);
    case 'get_tdelta_sec'
        ret= datevec(diff(TS.t1t2));
        ret= ret(6);
        
    otherwise
        error('inv op')
end

return; % end of main function


function TS= TS_ini()
TS= struct('mfname','', 't0',[], 't1t2',[], 'tu',[], 'tuInd',[], 'isKeys',0);


function TS= TS_load( TS, mfname, t1t2 )

% mfname is a matlab file like:
% function tu= mfname
% tu= [ t1 signal1 signal2 ... signalN;
%       ...
%       tM signal1 signal2 ... signalN ];

% load tu table
if ischar(mfname)
    TS.mfname= mfname;
    [p,f,~]= fileparts( mfname );
    cd0= cd;
    if ~isempty(p), cd(p); end
    TS.tu= eval(f);
    cd(cd0);
elseif isnumeric(mfname)
    TS.mfname= '';
    TS.tu= mfname;
else
    error('unexpected "mfname"')
end

% define the run time range
switch length(t1t2)
    case 2, TS.t1t2= t1t2;
    case 1, TS.t1t2= [t1t2 t1t2];
    case 0, TS.t1t2= [min(TS.tu(:,1)) max(TS.tu(:,1))];
    otherwise
        error('length t1t2 not 0, 1, 2')
end

% adjust t1t2 to be around "now"
tnow= now;
TS.t1t2= datenum( seconds(TS.t1t2) + tnow );

% adjust the tu table to be around "now"
% (must be done after t1t2 setting as t1t2 may need original tu(:,1)
TS.tu(:,1)= datenum( seconds(TS.tu(:,1)) +tnow );

% save starting time
TS.t0= tnow;
return


function [ret, tnow, tuInd]= TS_get_values( TS )

% define ind to start searching
ind= [];
if ~isempty(TS.tuInd)
    ind= TS.tuInd(end);
end
if isempty(ind)
    ind= 1;
end

% get the values
if isempty(TS.t1t2)
    % this should not have happen:
    error('TS.t1t2 is empty, forgot call "ini" of "time_signals"?');
end
tnow= now;
if tnow < min(TS.t1t2)
    % before time
    ret= TS.tu(1,2:end);
elseif max(TS.t1t2) <= tnow
    % after time
    ret= TS.tu(end,2:end);
    ind= size(TS.tu, 1);
else
    % within the table
    iMax= size(TS.tu,1);
    for i= ind:iMax
        if i==iMax
            ret= TS.tu(end,2:end);
            break;
        end
        if TS.tu(i,1)<=tnow && tnow<TS.tu(i+1,1)
            % found the time in the table
            ret= TS.tu(i,2:end);
            break;
        end
    end
    ind= i;
end

% prepare tuInd for future use
% length(tuInd) = 0 at start, 1 at 1st iter, 2 for the rest of time
if isempty(TS.tuInd)
    tuInd= ind;
else
    tuInd= [TS.tuInd(end) ind];
end

return


function [ret, ret2, tuInd]= TS_get_keys_and_timeouts( TS )
%
% ret= [k1 d1; k2 d2; ... kn dn]'
% ret2= tnow
% tuInd: 1x1 or 1x2

% get current keys
[keys, ret2, tuInd]= TS_get_values( TS );
ind= find(keys>0);

% if no nonzero cols then no keys
% if tuInd is the last one, then return no keys
% if current tu line equals the previous, then there are no new keys
if isempty(ind) || ...
        ( tuInd(end) == size(TS.tu,1) ) || ...
        ( length(tuInd)==2 && TS.tuInd(end)== tuInd(end) ) 
    ret= []; return
end

% tuInd changed, there are new keys, return their names and durations
% as tuInd is not the last one, return time difference
% translate keys 1:12 to [1:10 0 11] as needed by keyhandler()
i1= tuInd(end);
%dt= TS.tu(i1+1,1)-TS.tu(i1,1); % duration
to= TS.tu(i1+1,1); % timeout
LUT= [1:10 0 11];
%ret= [LUT(ind); zeros(1,length(ind))+dt];
ret= [LUT(ind); zeros(1,length(ind))+to];

return


% ----------------------------------------------------------------------
function mymenu( cmdId )
if nargin<1
    cmdId= '';
end

% List of available menu options
PCAC= 'plc_comm_addresses_config';
cmd= {
    'Comms show addr and ip',        'myterminal5_aux("addr_cnf_show");'; ...
    'Comms addr base set def',       [PCAC '( 100 );']; ...
    'Comms addr base set m0 mw0',    [PCAC '( 103 );']; ...
    'Comms addr base set m0 mw20',   [PCAC '( 104 );']; ...
    'Comms addr base set m0 mw180',  [PCAC '( 102 );']; ...
    'Comms addr base set m180 mw180',[PCAC '( 101 );']; ...
    'Comms addr base get cnf',       ['addrTbl= ' PCAC '( 0 )']; ...
    '---', ''; ...
    'PLC IP show', 'login_mapi_make_ip("show_ip2", mymodbus2_opt("ip"))'; ...
    'PLC IP make & set', 'mymodbus2_opt("set_ip", login_mapi_make_ip());'; ...
    'PLC IP set localhost (127.0.0.1)', 'mymodbus2_opt("set_ip", "127.0.0.1");'; ...
    ... %'PLC IP show (command line)', 'mymodbus2_opt("ip")'; ...
    ... %'PLC IP show (command line)', 'login_mapi_make_ip("show_ip1")'; ...
    ... %'PLC IP show (command line)', 'login_mapi_make_ip("show_ip2", mymodbus2_opt("ip"))'; ...
    'PLC IP change by hand', 'mymodbus2_opt("set_ip", login_mapi_make_ip("change_by_hand2", mymodbus2_opt("ip")));'; ...
    '---', ''; ...
    'Log final strings',     'myterminal_options( "setLogStrings", 2 );'; ...
    'Log tmp strings',       'myterminal_options( "setLogStrings", 1 );'; ...
    'Pause logging strings', 'myterminal_options( "setLogStrings", 0 );'; ...
    'Logged strings show',   'log_strings("show");'; ...
    'Log strings reset',     'log_strings("reset");'; ...
    '---', ''; ...
    'Key-down duration set 0.1sec', 'myterminal_options( "setKeyPressedDuration", 0.1 );'; ...
    'Key-down duration set 1sec',   'myterminal_options( "setKeyPressedDuration", 1 );'; ...
    'Key-down duration set 2sec',   'myterminal_options( "setKeyPressedDuration", 2 );'; ...
    'Input one key at a time',      'myterminal_options( "set", "accKeys", 0);'; ...
    'Input multiple keys (buffer)', 'myterminal_options( "set", "accKeys", 1);'; ...
    ... %'Accept multiple keys (do buffer)', ''; ...
    ... %'Show current options', 'myterminal_options( "showAll" );'; ...
    'Show current options', 'currentOptions= myterminal_options( "getAll" )'; ...
    '---', ''; ...
    ... %'Time table input signals inject', 'time_signals("ini_gui");'; ...
    'Time table input signals inject', 'myterminal5_aux("tu_run");'; ...
    'Time table of keys inject', 'myterminal5_aux("tu_keys_run");'; ...
    '---', ''; ...
    'LS debug ON',  'myterminal_options( "set", "logStringsDebug", 1 );'; ...
    'LS debug Off', 'myterminal_options( "set", "logStringsDebug", 0 );'; ...
    'Refresh debug ON',  'myterminal_options( "setRefreshDebug", 1 );'; ...
    'Refresh debug Off', 'myterminal_options( "setRefreshDebug", 0 );'; ...
    'Refresh period 1sec',   'myterminal_options( "setRefreshPeriod", 1 );'; ...
    'Refresh period 0.1sec', 'myterminal_options( "setRefreshPeriod", 0.1 );'; ...
    'Refresh period 0.01sec','myterminal_options( "setRefreshPeriod", 0.01 );'; ...
    'Refresh options show',  'refreshOptions= myterminal_options( "getAll" )'; ...
    '---', ''; ...
    ... %'Modbus debug ON',   'mymodbus2( "db_level_set", 1 )'; ...
    ... %'Modbus debug Off',  'mymodbus2( "db_level_set", 0 )'; ...
    'Modbus debug start',   'mymodbus2( "db_ini_data_log", 0 );'; ...
    'Modbus debug stop',    'mymodbus2( "db_end_data_log" );'; ...
    'Modbus debug reset',   'mymodbus2( "db_log_reset" );'; ...
    ... %'Modbus debug show', 'fprintf(1, "modbus dbLevel=%d\n", mymodbus2( "db_level", "get", [] ))'; ...
    'Modbus debug info',    'mymodbus2( "db_info" );'; ...
    };
for i=1:size(cmd,1)
    cmd{i,2}= strrep( cmd{i,2}, '"', '''' );
end

% Define what is to be done
if isempty(cmdId)
    % ask user what is to be done
    [indList, okFlag] = listdlg('PromptString',...
        {'Select action to do' ,'(press Cancel to avoid action):'},...
        'SelectionMode', 'multiple',... %'single', ... %'multiple',...
        'ListSize', [300 370], ...
        'ListString', cmd(:,1) );
else
    % cmdId should match an entry in cmd(:,1), e.g. cmd{5,1}
    okFlag= 0;
    for i=1:size(cmd,1)
        if strcmp(cmd{i,1}, cmdId)
            indList= i; okFlag= 1;
            break
        end
    end
end
if ~okFlag
    return
end
    
% Do the work
for i= indList
    if ~strcmp(cmd{i,1}, '---');
    eval( cmd{i,2} );
    end
end
