# Enhanced Online Sessions Plugin

### Lates Changes
Project Settings to customize the online subsystem used without having to change the DefaultEngine.ini

Unreal Engine Plugin used to manage an online subsystem <br>
Developed with Unreal Engine 5.3

EnhancedOnlineSessions Subsystem that helps communication with the online subsystem. <br>
Supports STEAM and EOS. <br>
Specific settings for EOS are also included.

Tired of the Advanced Sessions Plugin? <br>
This is the new one! <br>
Much more modular and should work right out of the box.

My goal was to make this plugin as modular as possible that it should work with any other project.

The entire online service logic is stored into a custom GameInstanceSubsystem.


## Supported actions:
<details>
  <summary>
    Login / Logout User
  </summary>
  <br>
    - Login/Logout to the online service with the given credentials. <br>
    - Required by EOS. <br>
    - Supports "Account Portal", "Developer", "External Auth", "Persistant Auth", and more.
</details>
<details>
  <summary>
    Finding and Creating Online/Offline Sessions
  </summary>
  <br>
    - Find sessions hosted in your online service. Or create a new one<br>
    - Supports LAN, Offline and Online sessions. <br>
    - Expanded Functionality for EOS (SEARCH_KEYWORDS, etc). <br>
</details>
<details>
  <summary>
    Friends
  </summary>
  <br>
    - Read the players friends list
</details>
