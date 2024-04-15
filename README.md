# Common Online Sessions Plugin
Unreal Engine Plugin used to manage an online subsystem <br>
Developed with Unreal Engine 5.3

CommonOnlineSessions Subsystem that helps communication with the online subsystem.
Supports STEAM and EOS
Specific settings for EOS are included.

Tired of the Advanced Sessions Plugin? <br>
This is the new one! <br>
Much more modular and should work right out of the box.

My goal was to make this plugin as modular as possible that it should work with any other project.

The entire online service logic is stored into a custom GameInstanceSubsystem.


## Supported actions:
<details>
  <summary>
    Login Common User
  </summary>
  <br>
    - Login to the online service with the given credentials. <br>
    - Required by EOS. <br>
    - Supports "Account Portal", "Developer", "External Auth", "Persistant Auth", and more.
</details>
<details>
  <summary>
    Find Online/Offline Sessions
  </summary>
  <br>
    - Find sessions hosted in your online service. <br>
    - Supports LAN, Offline and Online sessions. <br>
    - Expanded Functionality for EOS (SEARCH_KEYWORDS, etc). <br>
</details>
<details>
  <summary>
    Create Online/Offline Sessions
  </summary>
  <br>
    - Host sessions in your online service. <br>
    - Supports LAN, Offline and Online sessions. <br>
    - Expanded Functionality for EOS (SEARCH_KEYWORDS, etc). <br>
</details>
