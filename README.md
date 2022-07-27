# *DeepWiSim*: A Wireless Signal Simulator for Automatic Deep Learning

An tool can automatically generate wireless signal measurement and simultaneously train the deep learning models online.

[Demo Video](https://youtu.be/N3xNYJ51Lb0)

## Introduction

Today, deep learning has been used for wireless signal analysis in many applications. 
By collecting measurement data of wireless signals from the environment, deep learning models can be trained to accurately predict the change of signal strength in a dynamic environment. 
However, constructing high-quality training data by measuring wireless signals in real experiment environments is often labor-intensive and time-consuming, which is the biggest obstacle for applying the newest deep learning models to wireless
signal analysis. 
To address such issues, we create DeepWiSim, a wireless signal simulator that automates the deep learning process from training date generation to online model training and testing. 
In DeepWiSim, we design ray-tracing algorithms to simulate wireless signal propagation and use the [UE4](https://www.unrealengine.com/) game engine to model the dynamic environment. 
We integrate deep learning frameworks into the simulator as python background services that can automatically capture the signal measurement data from the environment and train the deep learning models online. 


## Getting Start

### Clone the project

```shell script
git clone https://github.com/codelzz/DeepWiSim.git
```

### Install Unreal Engine 4

1. [Download](https://www.unrealengine.com/en-US/download) and Install the UnrealEngine Launcher.
2. Open Unreal Engine Launcher and install UnrealEngine (>=4.26.2)

### Install Python3 Packages

1. Install required dependencies
```shell script
pip install -r requirements.txt
```

### Run Simulator

1. Open DeepWiSim.uproject file

2. When start the project for the first time, UE4 will pop up a panel says "Missing DeepWiSim Modules" please click Yes to rebuild the project. (You might encouter Error say "DeepWiSim could not be compiled.", please refer to [Troubles Shooting](https://github.com/codelzz/DeepWiSim#troubles-shooting))

3. Click Build button to build the project

4. Click Play button to run the simulator

### Run Deep Learning Service

1. In DeepWiSimPy (Source\DeepWiSimPy) directory run

```shell script
python app.py
```

### Run [JupyterLab](https://jupyter.org/) for Data Visualization

1. In DeepWiSimPy directory run
```shell script
jupyter lab
```

## Advanced Settings

### Simulator Settings

You can open the settings menu with the key `<P>`.
- `Control Mode`, controls the behavior of our target character.
	- `IDEL`, the target character will remain in his current position.
	- `Constant`, the target character will walk with a constant path.
	- `Random`, the target character will walk randomly.

	(*Notice*: the AI can only take control of the target when you are in GodMode. If you are [manually controlling the target](https://github.com/codelzz/DeepWiSim#simulated-target-control), you can press `<Tab>` switch back to GodMode and release the target control.)

- `Frequency (GHz)`, control the frequency of the simulated wireless signal. It is set to 2.45 GHz by default as simulating Bluetooth beacon signal.
- `Transmit Power (dBm)`, control the transmit power of the beacon
- `DPI`, control the resolution of wireless sensor, increase DPI will increase the number of rays for simulating signal propagation.
- `Ray-trace Distance (cm)`, controls how far the ray can reach.
- `Ray-trace Depth (cm)`, controls the number of times the ray can be reflected.

### Simulated Target Control

DeepWiSim also provides a way to generate data manually. After the simulator starts, you can press `<Tab>` to switch between `GodMode` and `PlayerMode`. In `PlayerMode` you can control the target to travel around the area. The simulator will keep sending measurement results to the deep learning service.

### Deep Learning Service Settings

In the settings.py (in Source\DeepWiSimPy) file, you can change the `APP_CONFIG` settings.

- `preprocessed_csv_file`, specifies the output measurement result.
- `checkpoint_path`, specifies where to store the deep learning model parameters.
- `training_history`, specifies where to store the training log.
- `training_epochs`, specifies how many epochs to run for the same dataset.
- `data_size`, specifies the number of rows to use for construct the training dataset. The deep learning service will always use the last `data_size` rows in `preprocessed_csv_file` for dataset construction.

## Troubles Shooting
- "DeepWiSim could not be compiled" when rebuilt the unreal project.
	This error might cause by missing  .NET Framework 4.8 SDK. Fix:
	1. [Download ](https://visualstudio.microsoft.com/downloads/#build-tools-for-visual-studio-2019) and install Build Tools for Visual Studio 2019
	2. After installed, the visual stuido installer will open. 
	3. Click "Individual components" on the top of navigation menu. 
	4. Select .NET Framework 4.8 SDK and click install.
	5. After installed the SDK, please re-open the unreal project files and continue the setup.
	
## Hints for UE4 Editor
- If you want to release UE4 control while running the simulator, you can press `<F8>` to eject and press the same key again to get back control.
