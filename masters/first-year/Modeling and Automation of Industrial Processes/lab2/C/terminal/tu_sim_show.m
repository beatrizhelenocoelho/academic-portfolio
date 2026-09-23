function tu_sim_show( tstId )
%
% Show results assuming mymodbus2.m run with debug on for a while
%
% Usage examples:
% tu_sim_show(0)
% tu_sim_show(1)
%
% Plot saved data:
% tu_sim_show(-1)
% tu_sim_show(-2)
% tu_sim_show(-12)

% April 2020, Dec2020 (show coils), JG

if nargin<1
    tstId= 0; %[-2 -1]; %-2; %-1; %-2; %0;
end
if length(tstId)>1
    for i= tstId, tu_sim_show(i); end
    return
end

switch tstId
    case 0, tu_sim_show_global_var(); % show Modbus timing data
    case 1, tu_sim_show_save;         % save Modbus timing data
    case 2, tu_sim_show_save('y');    % quiet save Modbus timing data

    case {-1, -2, -21, -22}
        % show data saved on files
        d= dir('*.mat');
        for i=1:length(d)
            if tstId==-1        % case -1
                figure(100+i); clf
                tu_sim_show_time_info( d(i).name );
            elseif tstId==-2    % case -2
                figure(200+i); clf
                tu_sim_show_coils( d(i).name, 'RW');
            elseif tstId==-21   % case -21
                figure(210+i); clf
                tu_sim_show_coils( d(i).name, 'W');
            elseif tstId==-22   % case -22
                figure(220+i); clf
                tu_sim_show_coils( d(i).name, 'R');
            end
        end
        
    case -12
        % show one file saved
        f= uigetfile('*.mat');
        if ischar(f)
            figure(11); clf
            tu_sim_show_time_info( f );
            figure(12); clf
            tu_sim_show_coils( f, 'RW' );
        end
        
end


% -----------------------------------------------------------------------
function tu_sim_show_global_var
global DBC
% reset: global DBC; DBC=[];

figure(200); clf
tu_sim_show_time_info( DBC )

% figure(201); clf
% tu_sim_show_coils( DBC, 'RW' )

figure(201); clf
subplot(211); tu_sim_show_coils( DBC, 'W' )
subplot(212); tu_sim_show_coils( DBC, 'R' )


function tu_sim_show_save( s )
global DBC
if nargin<1
    s= input('-- Save data to file? [yN] ', 's');
end
if strcmpi(s, 'y')
    t= datevec( now ); t(1)= rem(t(1),100); t(6)= round(t(6));
    ofname= sprintf('tu_sim_show_%02d%02d%02d_%02d%02d%02d.mat', t);
    try
        save( ofname, 'DBC' );
        DBC=[];
        fprintf(1, '   Saved data to file. Deleted data in memory.\n');
    catch
        fprintf(1, '** FAILED saving data to file.\n');
    end
end


function tu_sim_show_time_info( fname )
if ischar(fname)
    change_fig_title( fname );
    load( fname, 'DBC' );
else
    DBC= fname;
end

% myplot( DBC(1,:), DBC(2,:)-DBC(1,:), '.-' )

% [~,~,~,h1,m1,s1]= datevec( DBC(1,:) ); t1= 3600*h1 +60*m1 +s1;
% [~,~,~,h2,m2,s2]= datevec( DBC(2,:) ); t2= 3600*h2 +60*m2 +s2;
x= expand_DBC( DBC );
t1= x(1,:);
t2= x(2,:);

subplot(311)
plot( t1-t1(1), 1000*(t2-t1), '.-' )
xlabel('t [sec]')
ylabel('t_2-t_1 [msec]')
title('Duration (time) of each modbus command')

subplot(312)
plot( t1-t1(1), 1000*[diff(t1) 0], '.-' )
xlabel('t [sec]')
ylabel('t_1 diff [msec]')
title('Time differences between modbus commands')

subplot(313)
plot( t1-t1(1), min(x(3:end,:), 256)', '.-')
xlabel('target/writeFlag/length/addr vs t [sec]')

return


function tu_sim_show_coils( fname, opt )
if ischar(fname)
    change_fig_title( fname );
    load( fname, 'DBC' );
else
    DBC= fname;
end
x= expand_DBC( DBC );
% x= [t1; t2; c; f; n; a; v];

ind= find(x(3,:)=='c'+0); x= x(:,ind);
switch opt
    case 'R'
        ind= find(x(4,:)==0); x= x(:,ind);
        tstr= 'Terminal Read PLC coils';
    case 'W'
        ind= find(x(4,:)==1); x= x(:,ind);
        tstr= 'Terminal Write PLC coils';
    case 'RW'
        % keep all coils
        tstr= 'Terminal Read and Write PLC coils';
    otherwise
        error('inv opt')
end

t2= x(2,:);
c= x(3,:); % c=coil, h=register
f= x(4,:); % write flag
n= x(5,:); % number
a= x(6,:); % base address
v= x(7,:); % values

if 0
    v2= 0.7*de2bi(v);
    v2= v2+repmat(0:size(v2,2)-1, size(v2,1), 1);
    plot(t2-t2(1), v2, '.-')
else
    if isempty(v)
        warning('no data to show, selected data = %s', opt);
    else
        v2= de2bi(v);
        %v2= v2+repmat(a(:),1,size(v2,2));
        myplot( t2-t2(1), v2)
    end
end
xlabel('t [sec]')
title(tstr)


function change_fig_title( fname )
x= gcf;
str= sprintf( 'Fig%d "%s"', x.Number, fname );
set(gcf,'name',str,'numbertitle','off');


function x= expand_DBC( DBC )
% function debug_comms_log(writeFlag, t0, target, address, values)
% global DBC
% t= now; % time after modbus command (t0 is the time of sending)
% c= target(1)*10 +writeFlag;
% a= address(1);
% n= length(values);
% DBC(:,end+1)= [t0 t c n*1000+a]';

[~,~,~,h1,m1,s1]= datevec( DBC(1,:) ); t1= 3600*h1 +60*m1 +s1;
[~,~,~,h2,m2,s2]= datevec( DBC(2,:) ); t2= 3600*h2 +60*m2 +s2;
f= DBC(3,:);
c= floor(f/10); f= f-10*c;
a= DBC(4,:);
n= floor(a/1000); a= a-1000*n;
if size(DBC,1)>4
    v= DBC(5,:);
else
    v= zeros(size(n));
end
x= [t1; t2; c; f; n; a; v];


% -----------------------------------------------------------------------
function myplot(t, v, cstr)
% plot(t, v, cstr)
tu_sim( 'plot_z', [t(:) v], struct('clfFlag',0));
