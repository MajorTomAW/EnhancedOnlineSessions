
# Enhanced Online Sessions Plugin

An Unreal Engine Plugin used to manage an online subsystem  
Developed in Unreal Engine 5.3. Works on any Unreal Engine 5 Version.

Tired of the _Advanced Sessions Plugin_?  
This is the new one!  
Much more modular and should work right out of the box.

EnhancedOnlineSessions Subsystem that helps communicating with the online service.  
Supports **STEAM**, **Epic Online Services** (EOS) and **EOS Plus**.  
Including platform specific settings which can be changed inside the ``Editor.``

Well my goal was to make this plugin as modular as possible that it works with any other project.  
The entire online service logic is stored into a custom ```UGameInstanceSubsystem.```

> 💡 Suggestions or Feedback is much appreciated: [My Discord Account](https://discord.com/users/959875236403220490). Feel free to DM me any time! 🙂


<a name="documentation"></a>
# Documentation
> 1. __Setup__  
> &nbsp;1.1 [Configure your Project using the Online Subsystem of your Choice](#setup)  
> &nbsp;1.2 [Structure of the Enhanced Sessions](#structure)  
> &nbsp;1.3 [Session Maps](#session-maps)  
> 1. __Sessions__  
>       2.1 [Host Online Sessions](#online-sessions)  
>       &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;2.1.1 [Host Session](#host-session)  
>       &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;2.1.2 [Host Peer 2 Peer Lobby](#host-p2p-lobby)  
>       2.2 [Start Session](#start-session)  
>       2.3 [Find Sessions](#find-sessions)  
>       &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;2.3.1 [Get Current Players](#session-get-current-players)  
>       &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;2.3.1 [Get Max Players](#session-get-max-players)  
>       &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;2.3.1 [Get Session Friendly Name](#session-get-friendly-name)  
>       &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;2.3.1 [Get Ping in Ms](#session-get-ping)  
> 1. __Identity__  
>       &nbsp;3.1 [Login User](#login)  
>       &nbsp;3.2 [Kick Player](#kick-player)  
>       &nbsp;3.2 [Ban Player](#ban-player)  
>       &nbsp;3.2 [Change Player Name](#change-player-name)  
> 1. __Friends__  
>       &nbsp;3.2 [Coming Soon...](#friends)  

<a name="setup"></a>
# 1.1 Configure your Project
In ``Project Settings > Plugins`` there is a section called ``Online Enhanced Subsystem.``  
  
![image](https://github.com/MajorTomAW/EnhancedOnlineSessions/assets/96875345/19cb6a21-84a1-4311-bea7-6760974f1d31)

Here you can select the Online Subsystem of your Choice and configure its parameters.  
This will write the changes directly into the ``DefaultEngine.ini`` so you wont have to do that manually anymore :)  

> 📝You will be prompted to restart the Editor after.

<a name="structure"></a>
# 1.2 Structure of the Enhanced Sessions
> 💡The Enhanced Sessions Subsystem follows a consistent structure.  
All the necessary functions are stored inside the ``EnhancedOnlineSubsystem``  

![image](https://github.com/MajorTomAW/EnhancedOnlineSessions/assets/96875345/356ce3f8-b657-4a6b-92de-469b20b231dc)

These function will always take in a request object, which defines the action that you want to perform.  
Every request will have multiple callbacks, that you can bind functions to (Based on the request type).  
However, every request will have an ``OnFailedDelegate`` which will be called if the request fails and the log why the request may have failed will be passed through.  

<a name="session-maps"></a>
# 1.3 Session Maps with the Asset manager
If you want to use a Level for an Online Session you would need to tell the ``Asset Manager`` about its existence.  
To do that you would need to go inside your ``Project Settings -> Asset Manager``  

![Screenshot 2024-05-10 022213](https://github.com/MajorTomAW/EnhancedOnlineSessions/assets/96875345/1f8a52ef-1e61-43a5-b7ce-4b6062bf3112)  

And then make sure to uncheck ``Is Editor Only`` and add the Directory where your Level is located.  

![Screenshot 2024-05-10 022219](https://github.com/MajorTomAW/EnhancedOnlineSessions/assets/96875345/4a105936-6e99-42e4-9346-188c4b356654)  





<a name="online-sessions"></a>
# 2.1 Online Sessions
An online session is basically an instance of the game running on the server with a given set of properties which is either advertised so that it can be found and joined by players wanting to play the game or private so only players who are invited or notified of it in some way can join.  

The basic lifetime of a sessions is:
- Host a new session with desired settings.
- Wait for players to request to join the session.
- Register players who want to join.
- Start the session.
- Play the match.
- End Session or destroy the session.

To Host an Online Session you need to call the ``HostOnlineSession`` function which you can find in the ``EnhancedOnlineSubsystem.``  

![image](https://github.com/MajorTomAW/EnhancedOnlineSessions/assets/96875345/5a998405-cb43-406e-9ebb-cd1fab8ca8cf)

This takes in a ``HostSessionRequest.``  
There are two different types of sessions you can host:

<a name="host-session"></a>
## 2.1.1 Host Session
To host a Session you need to construct an ``Online Host Session Request.``  
Note that this will automatically travel to the map you selected in ``Map Id.``  
> :bulb: If you are not able to find your Map in ``Map Id`` checkout  [Session Maps](#session-maps)   and make sure you have setup everything correctly.

![image](https://github.com/MajorTomAW/EnhancedOnlineSessions/assets/96875345/a08a4774-d208-45d3-9d47-74d5ef39d08d)


| Parameter      | Description |
| ----------- | ----------- |
| __Online Mode:__ | Defines the online mode of the session. You can choose from: Offline / Online / LAN.  The Online Mode: "Online" adds ``?listen`` to the travel url so you would not need to add that manually to the ``Travel URL Operators``
| __Max Player Count__ | The maximum number of players that can be in the Session.
| __Map Id__ | Primary Asset Structure of the map you want to travel to, when the Session gets created.
| __Travel URL Operators__ | Adds additional operators that will be apended to the Travel URL.
| __Friendly Name__ | The friendly name of the session that will be used to display the Session name in the UI.
| __Search Keyword__ | The searchkeyword can be used for different scenarios. If you use EOS you will notice that this is the official ``SEARCH_KEYWORD`` combined with the Session. However you can also use it with any other online subsystem to filter your Session Results, depending on what Sessions you want to find
| __Use Lobbies if Available__ | Sets if the session should use player hosted Lobbies.
| __Use Voice Chat if Available__ | Some online services provide Voice Chat. This Settings defines if we use the Voice Chat.
| __Game Mode Advertisement Name__ | The advertisement name of the game mode that your session currently has. Can be used to advertise your session like _Team Deathmatch_, etc.
| __Is Presence__ | Defines if the hosted session should use Presence (Account Services)
| __Allow Join in Progress__ | Sets if other players can join the session when the session already is in progress. This can be used to disable joining when your game has been started.
| __On Failed Delegate__ | Delegate that will be called when the request failed for some reason.

<a name="host-p2p-lobby"></a>
## 2.1.2 Host Peer 2 Peer Lobby
To host a Peer 2 Peer Lobby you need to construct a ``Host P2P Lobby Request.``  
> 💡Note that this will NOT travel to a different map, as the P2P Lobby stays in the current level and you need to server travel manually.  

![image](https://github.com/MajorTomAW/EnhancedOnlineSessions/assets/96875345/8756fae9-c7b8-43ad-9c91-74f7317200ab)

Very similar to [Host Online Session](#host-session), this takes in a few parameters:  

| Parameter      | Description |
| ----------- | ----------- |
| __Online Mode__ | Defines the online mode of the session. You can choose from: Offline / Online / LAN.
| __Max Player Count__ | The maximum number of players that can be in the Session.
| __Friendly Name__ | The friendly name of the session that will be used to display the Session name in the UI.
| __Search Keyword__ | The searchkeyword can be used for different scenarios. If you use EOS you will notice that this is the official ``SEARCH_KEYWORD`` combined with the Session. However you can also use it with any other online subsystem to filter your Session Results, depending on what Sessions you want to find
| __Use Voice Chat if Available__ | Some online services provide Voice Chat. This Settings defines if we use the Voice Chat.
| __Game Mode Advertisement Name__ | The advertisement name of the game mode that your session currently has. Can be used to advertise your session like _Team Deathmatch_, etc.
| __Is Presence__ | Defines if the hosted session should use Presence (Account Services)
| __Allow Join in Progress__ | Sets if other players can join the session when the session already is in progress. This can be used to disable joining when your game has been started.
| __On Succeeded Delegate__ | This Delegate will be called whenever the lobby creation was a success.
| __On Failed Delegate__ | Delegate that will be called when the request failed for some reason.

Rest of the documenation is currently in progress...

<a name="start-session"></a>
# 2.2 Start Session
Starting a session basically sets its current state to ``InProgress`` meaning the the session is now registered as in progress in the online subsystem you are using.
This also blocks other players from joining if you disabled the option ``Allow Join in Progress`` in [Host Online Sessions.](#online-sessions)  
  
To start the session you would need to call the ``Start Online Session`` function which you can find in the ``Enhanced Online Subsystem``

![image](https://github.com/MajorTomAW/EnhancedOnlineSessions/assets/96875345/b5e73432-0f7f-4bb5-9e42-b25180584c08)

This takes in a ``Start Session Request.``  

![image](https://github.com/MajorTomAW/EnhancedOnlineSessions/assets/96875345/4d73cda8-0776-4602-9ab3-3d2ea15a27db)


| Parameter      | Description |
| ----------- | ----------- |
| __On Succeeded Delegate__ | Delegate that will be called when the Start Session Request succeeded and the current session is now set to be in progress.
| __On Failed Delegate__ | Delegate that will be called when the request failed for some reason.

As you can see this request does not take in any additional inputs, since you are only able to start the current session you are in.  
You could use the ``On Succeeded Delegate`` to start the match and a timer for example.  
> :memo: You dont **have** to start a session to start the match. Starting a session is more like a helper which the engine provides to help you doing the session management. Meaning that your game would still work fine if you dont start your session.  

<a name="find-sessions"></a>
# 2.3 Find Sessions
The most basic form of searching for sessions is the classic server browser that shows all available games and allows the player to filter those based on the type of game they want to play and then choose one to join one of the sessions that match those criteria.  
To find these sessions you would need to call the ``Find Online Sessions`` function which you can find in the ``Enhanced Online Subsystem.``   

![image](https://github.com/MajorTomAW/EnhancedOnlineSessions/assets/96875345/d35d48f8-05ee-444f-b3a1-69f116bbca1d)

This takes in a ``Find Sessions Request.``  

![image](https://github.com/MajorTomAW/EnhancedOnlineSessions/assets/96875345/b9b91bd6-99bf-4285-b93a-9ea649da26f4)

| Parameter      | Description |
| ----------- | ----------- |
| __Online Mode__ | Defines the online mode of the session you want to filter for. You can choose from: Offline / Online / LAN.
| __Max Search Results__ | The maximum number of sessions which can be found by the player requesting the find sessions.
| __Find Lobbies__ | Whether we want to find player hosted lobbies.
| __Search Keyword__ | Takes in an **optional** keyword if you want to filter the sessions you want to find. This Keyword can be assigned to a session in [Host Online Sessions.](#online-sessions)  
| __On Succeeded Delegate__ | Delegate that will be called when the Find Sessions Request succeeded and we have a list of found sessions.
| __On Failed Delegate__ | Delegate that will be called when the request failed for some reason.

<a name="session-get-current-players"></a>
## 2.3.1 Get Current Players
Returns the number of current players which are inside the session search result.  
![image](https://github.com/MajorTomAW/EnhancedOnlineSessions/assets/96875345/ed09b1da-e66d-4db4-b930-ff3856a8614f)

| Parameter      | Description |
| ----------- | ----------- |
| __Search Result__ | The session search result you want to read the current players from.

<a name="session-get-max-players"></a>
## 2.3.2 Get Max Players
Returns the maximum amount of players which could be possibly inside the session search result.  

![image](https://github.com/MajorTomAW/EnhancedOnlineSessions/assets/96875345/f55f3f4a-8b01-4745-9f7e-d3586638aa59)

| Parameter      | Description |
| ----------- | ----------- |
| __Search Result__ | The session search result you want to read the maxplayers from.

<a name="session-get-friendly-name"></a>
## 2.3.3 Get Session Friendly Name
Returns the friendly name of the session search result. The friendly name can be assigned to a session in [Host Online Sessions.](#online-sessions)  

![image](https://github.com/MajorTomAW/EnhancedOnlineSessions/assets/96875345/4aa9ea34-0ab7-42ea-9ede-2d84018e7057)

| Parameter      | Description |
| ----------- | ----------- |
| __Search Result__ | The session search result you want to read the friendly name from.

<a name="session-get-ping"></a>
## 2.3.4 Get Ping in Ms
Pings the session search result and returns the ping duration in ms.
> :memo: Some online subsystems do not support that action or require further work to make this working.
> 
![image](https://github.com/MajorTomAW/EnhancedOnlineSessions/assets/96875345/7d5dfef6-8f19-482a-9fa5-92fd0879bfb9)

| Parameter      | Description |
| ----------- | ----------- |
| __Search Result__ | The session search result you want to ping.
