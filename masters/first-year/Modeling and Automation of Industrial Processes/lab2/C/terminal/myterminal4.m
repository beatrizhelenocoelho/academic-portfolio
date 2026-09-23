function varargout = myterminal4(varargin)

% myterminal4 MATLAB code for myterminal4.fig
%      myterminal4, by itself, creates a new myterminal4 or raises the existing
%      singleton*.
%
%      H = myterminal4 returns the handle to a new myterminal4 or the handle to
%      the existing singleton*.
%
%      myterminal4('CALLBACK',hObject,eventData,handles,...) calls the local
%      function named CALLBACK in myterminal4.M with the given input arguments.
%
%      myterminal4('Property','Value',...) creates a new myterminal4 or raises the
%      existing singleton*.  Starting from the left, property value pairs are
%      applied to the GUI before myterminal4_OpeningFcn gets called.  An
%      unrecognized property name or invalid value makes property application
%      stop.  All inputs are passed to myterminal4_OpeningFcn via varargin.
%
%      *See GUI Options on GUIDE's Tools menu.  Choose "GUI allows only one
%      instance to run (singleton)".
%
% See also: GUIDE, GUIDATA, GUIHANDLES

% Edit the above text to modify the response to help myterminal4

% Last Modified by GUIDE v2.5 28-Mar-2020 11:23:44

% Begin initialization code - DO NOT EDIT
gui_Singleton = 1;
gui_State = struct('gui_Name',       mfilename, ...
                   'gui_Singleton',  gui_Singleton, ...
                   'gui_OpeningFcn', @myterminal4_OpeningFcn, ...
                   'gui_OutputFcn',  @myterminal4_OutputFcn, ...
                   'gui_LayoutFcn',  [] , ...
                   'gui_Callback',   []);
if nargin && ischar(varargin{1})
    gui_State.gui_Callback = str2func(varargin{1});
end

if nargout
    [varargout{1:nargout}] = gui_mainfcn(gui_State, varargin{:});
else
    gui_mainfcn(gui_State, varargin{:});
end
% End initialization code - DO NOT EDIT

% --- Executes just before myterminal4 is made visible.
function myterminal4_OpeningFcn(hObject, eventdata, handles, varargin)
% This function has no output args, see OutputFcn.
% hObject    handle to figure
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)
% varargin   command line arguments to myterminal4 (see VARARGIN)

% Choose default command line output for myterminal4
handles.output = hObject;

% Update handles structure
guidata(hObject, handles);

% save data to use later:
% myterminal4_aux('options', 'set', 'hObject', hObject); % not needed
myterminal4_aux('options', 'set', 'handles', handles);

% UIWAIT makes myterminal4 wait for user response (see UIRESUME)
% uiwait(handles.figure1);

% --- Outputs from this function are returned to the command line.
function varargout = myterminal4_OutputFcn(hObject, eventdata, handles)
% varargout  cell array for returning output args (see VARARGOUT);
% hObject    handle to figure
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)
% Get default command line output from handles structure
varargout{1} = handles.output;

function Untitled_1_Callback(hObject, eventdata, handles)
% disp('mymenu')
myterminal4_aux( 'mymenu', hObject, handles );

% --- Executes on button press in I_SWITCH_PRESENCE.
function I_SWITCH_PRESENCE_Callback(hObject, eventdata, handles)
% hObject    handle to I_SWITCH_PRESENCE (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)
% Hint: get(hObject,'Value') returns toggle state of I_SWITCH_PRESENCE
myterminal4_aux( 'set_coils', 0, get(hObject,'Value') );

% --- Executes on button press in I_SWITCH_ALARM.
function I_SWITCH_ALARM_Callback(hObject, eventdata, handles)
% hObject    handle to I_SWITCH_ALARM (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)
% Hint: get(hObject,'Value') returns toggle state of I_SWITCH_ALARM
myterminal4_aux( 'set_coils', 1, get(hObject,'Value') );

% --- Executes on button press in I_SWITCH_WINDOW.
function I_SWITCH_WINDOW_Callback(hObject, eventdata, handles)
% hObject    handle to I_SWITCH_WINDOW (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)
% Hint: get(hObject,'Value') returns toggle state of led_green
myterminal4_aux( 'set_coils', 2, get(hObject,'Value') );

% --- Executes on button press in key_.
function key_1_Callback(hObject, eventdata, handles)
% hObject    handle to key_ (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)
myterminal4_aux( 'key_push', 1 );

% --- Executes on button press in key_.
function key_2_Callback(hObject, eventdata, handles)
% hObject    handle to key_ (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)
myterminal4_aux( 'key_push', 2 );

% --- Executes on button press in key_.
function key_3_Callback(hObject, eventdata, handles)
% hObject    handle to key_ (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)
myterminal4_aux( 'key_push', 3 );

% --- Executes on button press in key_.
function key_4_Callback(hObject, eventdata, handles)
% hObject    handle to key_ (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)
myterminal4_aux( 'key_push', 4 );

% --- Executes on button press in key_.
function key_5_Callback(hObject, eventdata, handles)
% hObject    handle to key_ (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)
myterminal4_aux( 'key_push', 5 );

% --- Executes on button press in key_.
function key_6_Callback(hObject, eventdata, handles)
% hObject    handle to key_ (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)
myterminal4_aux( 'key_push', 6 );

% --- Executes on button press in key_.
function key_7_Callback(hObject, eventdata, handles)
% hObject    handle to key_ (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)
myterminal4_aux( 'key_push', 7 );

% --- Executes on button press in key_.
function key_8_Callback(hObject, eventdata, handles)
% hObject    handle to key_ (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)
myterminal4_aux( 'key_push', 8 );

% --- Executes on button press in key_.
function key_9_Callback(hObject, eventdata, handles)
% hObject    handle to key_ (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)
myterminal4_aux( 'key_push', 9 );

% --- Executes on button press in key_.
function key_10_Callback(hObject, eventdata, handles)
% hObject    handle to key_ (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)
myterminal4_aux( 'key_push', 10 );

% --- Executes on button press in key_.
function key_0_Callback(hObject, eventdata, handles)
% hObject    handle to key_ (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)
myterminal4_aux( 'key_push', 0 );

% --- Executes on button press in key_.
function key_11_Callback(hObject, eventdata, handles)
% hObject    handle to key_ (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)
myterminal4_aux( 'key_push', 11 );

% ---------------OUTPUTS---------------

% --- Executes on button press in GREEN_LED.
function GREEN_LED_Callback(hObject, eventdata, handles)
% hObject    handle to GREEN_LED (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)
% Hint: get(hObject,'Value') returns toggle state of GREEN_LED

% --- Executes on button press in YELLOW_LED.
function YELLOW_LED_Callback(hObject, eventdata, handles)
% hObject    handle to YELLOW_LED (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)
% Hint: get(hObject,'Value') returns toggle state of YELLOW_LED

% --- Executes on button press in RED_LED.
function RED_LED_Callback(hObject, eventdata, handles)
% hObject    handle to RED_LED (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)
% Hint: get(hObject,'Value') returns toggle state of RED_LED

% --- Executes on button press in BUZZER.
function BUZZER_Callback(hObject, eventdata, handles)
% hObject    handle to BUZZER (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)
% Hint: get(hObject,'Value') returns toggle state of BUZZER


% --- Executes on button press in REFRESH.
function REFRESH_Callback(hObject, eventdata, handles)
% hObject    handle to REFRESH (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)
myterminal4_aux( 'refresh', hObject, handles );


% --- Executes on button press in PASS_INCORRECT.
function PASS_INCORRECT_Callback(hObject, eventdata, handles)
% hObject    handle to PASS_INCORRECT (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)
% Hint: get(hObject,'Value') returns toggle state of PASS_INCORRECT


% --- Executes during object creation, after setting all properties.
function edit7_CreateFcn(hObject, eventdata, handles)
% hObject    handle to edit7 (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    empty - handles not created until after all CreateFcns called
% Hint: edit controls usually have a white background on Windows.
%       See ISPC and COMPUTER.
if ispc && isequal(get(hObject,'BackgroundColor'), get(0,'defaultUicontrolBackgroundColor'))
    set(hObject,'BackgroundColor','white');
end

function edit7_Callback(hObject, eventdata, handles)
% hObject    handle to edit7 (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)
% Hints: get(hObject,'String') returns contents of edit7 as text
%        str2double(get(hObject,'String')) returns contents of edit7 as a double

% --- Executes during object deletion, before destroying properties.
function edit7_DeleteFcn(hObject, eventdata, handles)
% hObject    handle to edit7 (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% --- Executes during object creation, after setting all properties.
function edit8_CreateFcn(hObject, eventdata, handles)
% hObject    handle to edit8 (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    empty - handles not created until after all CreateFcns called
% Hint: edit controls usually have a white background on Windows.
%       See ISPC and COMPUTER.
if ispc && isequal(get(hObject,'BackgroundColor'), get(0,'defaultUicontrolBackgroundColor'))
    set(hObject,'BackgroundColor','white');
end

function edit8_Callback(hObject, eventdata, handles)
% hObject    handle to edit8 (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)
% Hints: get(hObject,'String') returns contents of edit8 as text
%        str2double(get(hObject,'String')) returns contents of edit8 as a double
disp('edit8_Callback')

% --- Executes during object creation, after setting all properties.
function edit9_CreateFcn(hObject, eventdata, handles)
% hObject    handle to edit9 (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    empty - handles not created until after all CreateFcns called
% Hint: edit controls usually have a white background on Windows.
%       See ISPC and COMPUTER.
if ispc && isequal(get(hObject,'BackgroundColor'), get(0,'defaultUicontrolBackgroundColor'))
    set(hObject,'BackgroundColor','white');
end

function edit9_Callback(hObject, eventdata, handles)
% hObject    handle to edit9 (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)
% Hints: get(hObject,'String') returns contents of edit9 as text
%        str2double(get(hObject,'String')) returns contents of edit9 as a double
