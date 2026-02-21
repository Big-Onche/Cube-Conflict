# Building and launching a server via terminal (GNU/Linux method)
### ⚙️ 1. Connect to your server
`ssh root@YOUR.SERVER.IP`
### 🧰 2. Install dependencies
`sudo apt update`

`sudo apt -y install git build-essential pkg-config zlib1g-dev libenet-dev libcurl4-openssl-dev libopenal-dev libsdl2-dev libsdl2-image-dev libpng-dev libsndfile1-dev tmux ufw`

* build-essential – compiler tools
* libenet-dev / zlib1g-dev – networking + compression
* libsdl2, libopenal-dev, libsndfile1-dev – graphics/audio deps (needed to compile even for server)
* ufw / tmux – firewall + optional terminal session tool

### 📦 3. Clone & build Cube Conflict
`cd /opt`

`sudo git clone --depth=1 https://github.com/Big-Onche/Cube-Conflict.git`

`sudo chown -R "$USER":"$USER" Cube-Conflict`

`cd Cube-Conflict`

` make -j"$(nproc)" -C src server`

### ⚙️ 4. Configure the server
Edit your server config:

`nano config/server-init.cfg`

Set:

`serverip  YOUR.SERVER.IP` (Uncomment the line and put your server's ip)

`serverport 43000` (Uncomment the line and set your port)

### 🔒 5. Open firewall ports
`sudo ufw allow 22/tcp` (keep SSH open)

`sudo ufw allow 43000/udp` (open Cube Conflict default port)

`sudo ufw enable`

### 🧪 6. (Optionnal) Test the server manually

`cd /opt/Cube-Conflict`

`LD_LIBRARY_PATH=./bin_unix ./bin_unix/cc_server -d"./"`

You should see:

`Server started | Your.Super.Ip | Port: 43000`

Stop with Ctrl + C.

### 👤 7. Enable permissions for launch script
`sudo chmod +x /opt/Cube-Conflict/run.sh`

### 👤 8. (If not already done) Create a non-root service user
`sudo useradd -r -m -s /usr/sbin/nologin mycubeserv || true`

`sudo chown -R mycubeserv:mycubeserv /opt/Cube-Conflict`

### 🧩 9. Create the systemd service
```json
sudo tee /etc/systemd/system/cube-conflict.service >/dev/null <<'EOF'
[Unit]
Description=Cube Conflict Dedicated Server
After=network-online.target
Wants=network-online.target

[Service]
User=cubeserv
Group=cubeserv
WorkingDirectory=/opt/Cube-Conflict
ExecStart=/opt/Cube-Conflict/run.sh server
Restart=always
RestartSec=5
LimitNOFILE=65535
NoNewPrivileges=true
PrivateTmp=true
ProtectSystem=full
ProtectHome=true

[Install]
WantedBy=multi-user.target
EOF

```

### 🚀 10. Enable autostart

`sudo systemctl daemon-reload`

`sudo systemctl enable cube-conflict`

`sudo systemctl start cube-conflict`

### 🔍 11. Verify operation
#### View live logs
`sudo journalctl -u cube-conflict -f`

#### Check UDP socket
`ss -lunp | grep 43000`

#### See server logs
`ls -l /opt/Cube-Conflict/logs`

`tail -f /opt/Cube-Conflict/logs/server_*.log`

#### You should see:
`Server started | ... | Port: 43000`

` ... `

### ✅ 12. Reboot & auto-start check
`sudo reboot`

#### then reconnect
`sudo systemctl status cube-conflict --no-pager`

`tail -f /opt/Cube-Conflict/logs/server_*.log`

## Edit server settings 

You need to edit [config](../config/server-init.cfg) file (`config/server-init.cfg`).

| Param             | Defalt value                          | Description                                                                                           |
| :---              | :---                                  | :---                                                                                                  |
| serverip          | 0.0.0.0                               | Specific ip for server to use (must be the IP of your machine!)                                                                         |
| serverport        | 43000                                 | Specific port for server to use (keep it to default, or change it if you want, if you want to run multiple servers on your machine, use another port)                                                                      |
| updatemaster        | 0 or 1                                 | Controls whether or not the server reports to the public masterserver (0 = disabled | 1 = enabled) port)                                                                      |
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
| publicserver        | 0                                      | Controls whether or not the server is intended for "public" use : when set to 0, allows "setmaster 1" and locked/private mastermodes (for coop-editing and such), when set to 1, can only gain master by "auth" or admin, and doesn't allow locked/private mastermodes, when set to 2, allows "setmaster 1" but disallows private mastermode (for public coop-editing)                                                            |
| serverdescfr      | Mon serveur Cube Conflict             | Server description shows for the server browser (French)                                              |
| serverdescen      | My Cube Conflict server               | Server description shows for the server browser (English)                                             |
| servermotd        |                                       | Message of the day to send to players on connect (opt.)                                               |
| restrictdemos     | 1                                     | Controls whether admin privs are necessary to record a demo (1 -> admin, 2 -> master)                 |
| maxdemos          | 5                                     | Max num of demos the server will store                                                                |
| maxdemosize       | 16                                    | Max size a demo is allowed to grow to in Mb                                                           |                                |
| restrictpausegame | 1                                     | Controls whether admin privs are necessary to pause a game (1 -> admin, 0 -> master)                  |
| lockmaprotation   | 2                                     | Whether or not to allow players to vote on maps not in rotation (2 -> admin, 1 -> master, 0 -> any)   |
| maps              | [village factory moon castle volcano island] | List of playable maps                                                                                 |

* `maprotationreset`
* `maprotation "*" $maps`

* `teamkillkickreset`
* `teamkillkick "*" 7 30`
* `teamkillkick "?capture" 10 30`


