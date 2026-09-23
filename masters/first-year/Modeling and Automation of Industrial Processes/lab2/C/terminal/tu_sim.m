function tu= tu_sim( op, a1, options )
%
% Interact with UnityPro given a tu (time table) - UNDER CONSTRUCTION
%
% Currently implemented:
% Given a timed list of keys, generate a tu table
% i.e. table where column 1 is time and next columns are flags
% 
% Inputs:
% op: str : command string 'make_tu', 'nbits_one_by_one' or 'plot_z'
% a1: ... : input argument, depends on op
%
% Outputs:
% tu: NxM : [t1 k1 k2 ... kn; t2 k1 k2 ... kn; ...]

% Usage examples:
% tu= tu_sim('make_tu',  {[0], [1 1], [2]})
% tu_sim('plot_z', tu_sim('make_tu',  {[0], [1 1], [2]}) )

% An example for "PN_device_kb_IO.m":
% tkList= {[0], [1 1], [2], [3 5], [4], [5 9], [6], [7 9 12], [8 12], [9]};
% tu= tu_sim('make_tu', tkList);
% tu_sim('plot_z', tu)

% 15.3.2021, 4.4.2021 (new fn name, cmts++), J. Gaspar

if nargin<3
    options= [];
end

switch op
    case 'make_tu'
        % set the number of bits given the max key number
        tkList= a1;
        tu= expand_list_to_array( tkList, options );

    case 'nbits_one_by_one'
        % set the number of bits, i.e. size(tu,2)= 1+nBits
        nBits= a1;
        tkList= circle_N_bits( nBits )
        tu= expand_list_to_array( tkList, options );
        
    case 'plot_z'
        % plot_z.m is required for "sharp_plot_z"
        tu= a1;
        sharp_plot_z( tu, options );

    otherwise
        error('inv op')
end

return


% -----------------------------------------------------------------------
function tu= expand_list_to_array( tkList, options )

if ~isfield(options, 'nBits')
    % "nBits" is not mandatory, but is nicer
    nBits= 0;
    for i= 1:length(tkList)
        tk= tkList{i};
        keys= tk(2:end);
        if ~isempty(keys)
            nBits= max(nBits, max(keys));
        end
    end
    options.nBits= nBits;
end

tu= [];
for i= 1:length(tkList)
    tk= tkList{i};
    keys= mk_keys(tk(2:end), options);
    tu= vertcat_enlarge( tu, [tk(1) keys] );
end

return


function y= mk_keys(kid, options)

nBits= 9; %12;
if isfield(options, 'nBits')
    nBits= options.nBits;
end
y= zeros(1, nBits);

for i=1:length(kid)
    y(kid(i))= 1;
end


function tu= vertcat_enlarge( tu, newLine )
if nargin<1
    vertcat_enlarge_demo
    return
end

%x= newLine(:)';
x= newLine;

su1= size(tu,1);
su2= size(tu,2);
sx1= size(x,1);
sx2= size(x,2);

if isempty(tu)
    tu= x;
    return
end

if su2==sx2
    % matching number of cols
    tu= [tu; x];
elseif su2 < sx2
    % newLine has too many cols
    tu= [tu zeros(su1, sx2-su2); x];
else % su2 > sx2
    % newLine has too few cols
    tu= [tu; x zeros(sx1, su2-sx2)];
end


function vertcat_enlarge_demo
%vertcat_enlarge( 1:3, 1:2 )
%vertcat_enlarge( 1:3, eye(2) )
vertcat_enlarge( eye(2), 1:3 )


function tkList= circle_N_bits( nBits )
if nargin<1
    nBits= 10;
end
tkList= {};
for i=1:nBits
    tkList{end+1}= [i-1 i];
end
tkList{end+1}= nBits;
return


% -----------------------------------------------------------------------
function sharp_plot_z( tu, options )
% Keyboard keys seen by lines:
%  1  2  3
%  4  5  6
%  7  8  9
% 10 11 12

tu= kron( tu, ones(2,1) );
% for i=3:2:size(tu,1)
%     tu(i,2:end)= tu(i-1,2:end);
% end
tu(2:2:end-1,1)= tu(3:2:end-1,1);

%figure(201); clf
plot_z( tu(:,1), tu(:,2:end), options )
ylabel('input bits')
xlabel('time [seconds]')
title('Input bits vs time')


function plot_z(t, z, options)
% Show various time signals, collected as columns z(:,i)
%
% t : Nx1 : time range
% z : Nxm : signals to show
%
% cstr: string : (facultative) plot modifier

% Nov2013, Nov2015 (columns), JG

if nargin<3
    options= [];
end
if ischar(options)
    cstr= options;
elseif isfield(options, 'cstr')
    cstr= options.cstr;
else
    cstr= '.-';
end

if isempty(t)
    x= (1:size(z,2))';
else
    x= t(:);
end
if ~isfield(options, 'plot3')
    dy= 0.5*z/max(max(abs(z))); % used for plot2
end

washold= ishold;
if ~washold
    clfFlag= 1;
    if isfield(options, 'clfFlag')
        clfFlag= options.clfFlag;
    end
    if clfFlag
        clf
    end
end

hold on
for i=1:size(z,2)
    y= i+x*0;
    if isfield(options, 'plot3')
        plot3(x, y, z(:,i), cstr)
    else
        plot(x, y+dy(:,i), cstr);
    end
end
if ~washold
    hold off
end
