# How to launch server via terminal (GNU/Linux method)

1. You need to [build](../bin_unix/readme.md) the game.
2. Use `ccunix.sh` with `-d` flag.
3. Now you can connect to server via game client (`ccunix.sh` without `-d` flag).

## Edit server settings 

You need to edit [config](../config/server-init.cfg) file (`config/server-init.cfg`).

| Param             | Defalt value                          | Description                                                                                           |
| :---              | :---                                  | :---                                                                                                  |
| serverip          | 0.0.0.0                               | Specific ip for server to use                                                                         |
| serverport        | 43000                                 | Specific port for server to use                                                                       |
| maxclients        | 20                                    | Max number of allowed clients                                                                         | 
| gamelength        | 10                                    | Game duration in minutes                                                                              | 
| teamkill          | 1                                     | Activate or deactivate teamkill (0 -> disabled, 1 -> enabled)                                         |
| servaddbots       | 0                                     | Automatically add bots up to the min amount of players                                                |
| servbotminskill   | 60                                    | Min level of bots                                                                                     |
| servbotmaxskill   | 90                                    | Max level of bots                                                                                     |
| servrandommode    | 0                                     | Randomize game modes                                                                                  |
| servforcemode     | -1                                    | Choose a fixed game mode (-1 -> disabled, 1 -> coop map editor, 2-18 -> to choose the game mode)      |
| adminpass         | PASSWORD123                           | Admin password, used in `/setmaster password_here` command. **Change before startup server!**         |
| serverpass        |                                       | Password required to connect to the server                                                            |
| serverdescfr      | Mon serveur Cube Conflict             | Server description shows for the server browser (French)                                              |
| serverdescen      | My Cube Conflict server               | Server description shows for the server browser (English)                                             |
| servermotd        |                                       | Message of the day to send to players on connect (opt.)                                               |
| restrictdemos     | 1                                     | Controls whether admin privs are necessary to record a demo (1 -> admin, 2 -> master)                 |
| maxdemos          | 5                                     | Max num of demos the server will store                                                                |
| maxdemosize       | 16                                    | Max size a demo is allowed to grow to in Mb                                                           |
| updatemaster      | 0                                     | Controls whether or not the server reports to the masterserver                                        |
| restrictpausegame | 1                                     | Controls whether admin privs are necessary to pause a game (1 -> admin, 0 -> master)                  |
| lockmaprotation   | 2                                     | Whether or not to allow players to vote on maps not in rotation (2 -> admin, 1 -> master, 0 -> any)   |
| maps              | [village factory moon castle volcano] | List of playable maps                                                                                 |

* `maprotationreset`
* `maprotation "*" $maps`

* `teamkillkickreset`
* `teamkillkick "*" 7 30`
* `teamkillkick "?capture" 10 30`


