function [driverPath, x0, y0, v0, yaw0, simStopTime] = ...
    createDriverPath(scenario,egoID,subsample)
% Create driver path

% v0    % Initial speed of the ego car           (m/s)
% x0    % Initial x position of ego car          (m)
% y0    % Initial y position of ego car          (m)
% yaw0  % Initial yaw angle of ego car           (degrees)

if nargin < 2
    subsample = 4;
end

% Extract ego pose information
v0 = norm(scenario.Actors(egoID).Velocity(:));
%x0 = scenario.Actors(egoID).Position(1);
%y0 = scenario.Actors(egoID).Position(2);
%yaw0 = deg2rad(scenario.Actors(egoID).Yaw);
% change: new starting position and yaw angle to match the one from the 3D example

% position 1:
%x0 = -11.09;
%y0 = -141.9;
%yaw0 = 0;

% position 2:
x0 = -890.1;
y0 = 1.4320e+03;
yaw0 = -7;

% Driver path is a subsampled version of ego poses
restart(scenario);
poses = record(scenario);
numPoints = floor(numel(poses)/subsample);

driverPath = zeros(numPoints,2);
for n = 1:numPoints
    poseIndex = 1 + (n-1) * subsample;
    driverPath(n,:) = poses(poseIndex).ActorPoses(1).Position(1:2);
end

%simStopTime = poses(poseIndex).SimulationTime;
simStopTime = 30; % change: dictates the actual simulation run time