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

💡 Suggestions or Feedback is much appreciated: [My Discord Account](https://discord.com/users/959875236403220490). Feel free to DM me any time! 🙂
<a name="documentation"></a>
# Documentation
> 1. __Setup__  
> &nbsp;1.1 [Configure your Project using the Online Subsystem of your Choice](#setup)  
> &nbsp;1.2 [Structure of the Enhanced Sessions](#structure)  
> 1. __Sessions__  
>       2.1 Host Online Sessions  
>       &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;2.1.1 [Host Session](#host-session)  
>       &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;2.1.2 [Host Peer 2 Peer Lobby](#host-p2p-lobby)  
>       2.2 [Start Session](#start-session)  
>       2.3 [Find Sessions](#find-sessions)  
> 1. __Identity__  
>       &nbsp;3.1 [Login User](#login)  

<a name="setup"></a>
# 1.1 Configure your Project using the Online Subsystem of your Choice
In ``Project Settings > Plugins`` there is a section called ``Online Enhanced Subsystem.``  
  
![image](https://github.com/MajorTomAW/EnhancedOnlineSessions/assets/96875345/19cb6a21-84a1-4311-bea7-6760974f1d31)

Here you can select the Online Subsystem of your Choice and configure its parameters.  
This will write the changes directly into the ``DefaultEngine.ini`` so you wont have to do that manually anymore :)  

📝You will be prompted to restart the Editor after.

<a name="structure"></a>
# 1.2 Structure of the Enhanced Sessions
💡The Enhanced Sessions Subsystem follows a consistent structure.  
All the necessary functions are stored inside the ``EnhancedOnlineSubsystem``  

![image](https://github.com/MajorTomAW/EnhancedOnlineSessions/assets/96875345/356ce3f8-b657-4a6b-92de-469b20b231dc)

These function will always take in a request which defines the action that you want to do.  
Every request will have multiple callabacks, based on the request type, that you can bind functions to.  
However, every request will have an ``OnFailedDelegate`` which will be called if the request fails and the log why the request may have failed will be passed through the delegate.


# 2.1 Online Sessions
To Host an Online Session you need to call the ``HostOnlineSession`` function which you can find in the ``EnhancedOnlineSubsystem``
This takes in a ``HostSessionRequest.`` There are two different types of session you can host:

<a name="host-session"></a>
## 2.1.1 Host Session
To host a Session you need to construct an ``OnlineHostSessionRequest.``  

![image](https://github.com/MajorTomAW/EnhancedOnlineSessions/assets/96875345/9c594d71-6ae7-4f2d-9a4b-c8a892f28eb1)

| Parameter      | Description |
| ----------- | ----------- |
| __Online Mode__ | Defines the online mode of the session. Offline/Online/LAN.
| __Max Player Count:__ | The maximum number of players that can be in the Session.
| __Map Id:__ | Primary Asset Structure of the map you want to travel to, when the Session gets created.
| __Friendly Name:__ | The friendly name of the session that will be used to display the Session name in the UI.
| __Search Keyword:__ | The searchkeyword can be used for different szenarios. If you use EOS you will notice that this is the official ``SEARCH_KEYWORD`` combined with the Session. However you can also use it with any other online subsystem to filter your Session Results, depending on what Sessions you want to find
| __Use Lobbies if Available:__ | Sets if the session should use player hosted Lobbies.
| __Use Voice Chat if Available:__ | Some online services provide Voice Chat. This Settings defines if we use the Voice Chat.
| __Game Mode Advertisement Name:__ | The advertisement name of the game mode that your session currently has. Can be used to advertise your session like _Team Deathmatch_, etc.
| __Is Presence:__ | Defines if the hosted session should use Presence (Account Services)
| __Allow Join in Progress:__ | Sets if other players can join the session when the session already is in progress. This can be used to disable joining when your game has been started.

<a name="host-p2p-lobby"></a>
## 2.1.2 Host Peer 2 Peer Lobby
To host a Peer 2 Peer Lobby you need to construct an ``OnlineHostP2PLobbyRequest.``  

![image](https://github.com/MajorTomAW/EnhancedOnlineSessions/assets/96875345/8756fae9-c7b8-43ad-9c91-74f7317200ab)

Very similar to [Host Online Session](#host-session), this takes in a few parameters:  

| Parameter      | Description |
| ----------- | ----------- |
| __Online Mode__ | Defines the online mode of the session. Offline/Online/LAN.
| __Max Player Count:__ | The maximum number of players that can be in the Session.
| __Friendly Name:__ | The friendly name of the session that will be used to display the Session name in the UI.
| __Search Keyword:__ | The searchkeyword can be used for different szenarios. If you use EOS you will notice that this is the official ``SEARCH_KEYWORD`` combined with the Session. However you can also use it with any other online subsystem to filter your Session Results, depending on what Sessions you want to find
| __Use Voice Chat if Available:__ | Some online services provide Voice Chat. This Settings defines if we use the Voice Chat.
| __Game Mode Advertisement Name:__ | The advertisement name of the game mode that your session currently has. Can be used to advertise your session like _Team Deathmatch_, etc.
| __Is Presence:__ | Defines if the hosted session should use Presence (Account Services)
| __Allow Join in Progress:__ | Sets if other players can join the session when the session already is in progress. This can be used to disable joining when your game has been started.
| __On Succeeded Delegate:__ | This Delegate will be called whenever the lobby creation was a success.
