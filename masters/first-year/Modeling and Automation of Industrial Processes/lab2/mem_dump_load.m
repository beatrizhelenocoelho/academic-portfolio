function x= mem_dump_load( fname, options )
% Show two memory dumps created with Unity
% Oct2014, JG

if nargin<2
    options= [];
end

fid= fopen(fname, 'rb');
x= fread(fid, inf, 'uint16');
fclose(fid);

x= select_data(x, options);

return


function x= select_data(x, options)

% find how to read data
if ~isfield(options, 'iniWord') && ~isfield(options, 'endWord') && ...
        ~isfield(options, 'noAutoSearch')
    x= find_data_location_and_size( x, options );
else
    iniWord= 1; if isfield(options, 'iniWord'); iniWord= options.iniWord; end
    endWord= inf; if isfield(options, 'endWord'); endWord= options.endWord; end
    x= truncate_data( x, iniWord, endWord )
end

return


function x= truncate_data( x, iniWord, endWord )

% truncate data if needed
if iniWord~=1 || ~isinf(endWord)
    if isinf(endWord) || endWord>length(x);
        endWord= length(x);
    end
    if iniWord<1 || iniWord>length(x)
        endWord= 1;
    end
    x= x(iniWord:endWord);
end


function x= find_data_location_and_size( x, options )
% find 12345 two times
ind= find(x==12345);
if isempty(ind)
    warning('data init flag not found');
    return
end

% find a place where 12345 appears at 2 consecutive locations
ind2= find(diff(ind)==1);
if isempty(ind2)
    warning('data init flag not found');
    return
elseif length(ind2)>1
    warning('data init flag found multiple times')
end
ind3= ind(ind2(1));

% header has the form [12345 12345 nnmmm]  nn=numLines  mmm=numColumns
header= x(ind3:ind3+2);

% buffer size [nLines nColumns], typicaly nLines=2
sz= header(3);
sz= [max(1,round(sz/1000)) rem(sz,1000)];

% try to cut out unused buffer area
if isfield(options, 'cropAsIndex') && options.cropAsIndex
    % use the index just before the 12345
    cnt= x(ind3-1);
    if cnt==0
        warning('Zero index. Maybe it is an old file. Doing nothing.');
    else
        nLines= sz(1);
        sz(2)= cnt/nLines;
    end
end

% reshape and return
x= x(ind3+3:ind3+3+prod(sz)-1);
x= reshape(x, sz);

return
