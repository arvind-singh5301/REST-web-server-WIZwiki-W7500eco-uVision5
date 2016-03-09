# RESTful Web Server & I/O Control by REST API

<p align="center">
  <img width="40%" src="https://ericslabs.files.wordpress.com/2016/03/wizwiki-w7500eco3dtop_edit.png" />
</p>

Simple REST Web server library for small IoT devices. 
Users can be made the IoT device for REST-based web services available in this project using C language / WIZwiki-W7500ECO platform board.

## Target Board and IDE
### Target Board
#### WIZWiki-W7500ECO
##### Pic
<!-- WIZwiki-W7500ECO pic -->
<p align="center">
  <img width="45%" src="https://ericslabs.files.wordpress.com/2016/03/eco-net-400.jpg" />
</p>
##### Pinout
<!-- WIZwiki-W7500ECO pinout -->
<p align="center">
  <img width="80%" src="http://wizwiki.net/wiki/lib/exe/fetch.php?media=products:wizwiki-w7500eco:wizwiki-w7500eco_detailpinout.png" />
</p>

For more details, please refer to [WIZ550web Wiki page](http://wizwiki.net/wiki/doku.php?id=products:wizwiki-w7500eco:start) in [WIZnet Wiki](http://wizwiki.net).

### Development Environment
 - Keil uVision5 IDE v5.10

## Build and Firmware Update
Build and Firmware write to target board: WIZwiki-W7500ECO

### Project Build
KEIL uVision5 project file can be found in below file path.
 - [$Projects/HTTP_Server_RESTAPI/W7500x_App.uvproj](https://github.com/hkjung/REST-web-server-WIZwiki-W7500eco-uVision5/tree/master/Projects/HTTP_Server_RESTAPI)

### Firmware Write
 - Writing firmware into WIZwiki-W7500ECO has three ways
   - Using the CMSIS-DAP (Drag and Drop)
   - Using the CMSIS-DAP (through KEIL uVision5)
   - Using the ISP (ISP, In-System Programming)

For more details, please refer to WIZnet wiki site.
 - [How to write the firmware into WIZwiki-W7500ECO page](http://wizwiki.net/wiki/doku.php?id=products:wizwiki_w7500eco:start_getting_started:write_firmware)


## REST API Design

* All resources(URI) are represented in lower case letters.
* REST API Document is under construction. It will be update continuously.


- - - 
### Symbols
[:id] : pre-defined 4-I/O pins, 'a', 'b', 'c', 'd'.
  - 'a' : P30 pin (digital input / digital output / analog input)
  - 'b' : P29 pin (digital input / digital output / analog input)
  - 'c' : P28 pin (digital input / digital output / analog input)
  - 'd' : P27 pin (digital input / digital output / analog input)

- - - 

### URI: HTTP GET method
##### index
```
http://w7500xRESTAPI.local/index
```
 - Controllable all IO pins and Resources list includes URL, HTTP method and simple descriptions

##### uptime
```
http://w7500xRESTAPI.local/uptime
```
 - Device uptime

##### netinfo
```
http://w7500xRESTAPI.local/netinfo
```
 - Network information (e.g., MAC / IP address ...)

##### userio
```
http://w7500xRESTAPI.local/userio
```
 - All active user IO's ID / Type (Digital or Analog) / Direction (Input or Output)

##### userio/id
```
http://w7500xRESTAPI.local/userio/:id
```
 - Get the User IO's status or value

##### userio/id/info
```
http://w7500xRESTAPI.local/userio/:id/info
```
 - Get the user IO's ID / Type / Direction

- - - 

### URI: HTTP POST method
##### userio/id
```
http://w7500xRESTAPI.local/userio/:id
```
 - Activate (Create) the specified IO

- - - 

### URI: HTTP PUT method* (in development)
##### userio/id
```
http://w7500xRESTAPI.local/userio/:id
```
 - Set (change) the User IO's status (digital output only)

##### userio/id
```
http://w7500xRESTAPI.local/userio/:id/info
```
 - Set (change) the User IO's Type / Direction

- - - 

### URI: HTTP DELETE method
##### userio/id
```
http://w7500xRESTAPI.local/userio/:id
```
 - Deactivate (Delete) the specified IO

- - - 

## Testing
Connect your board to your network and run test tool. These library has been tested on Postman Builder.

For more details about Postman, please refer to [Postman Chrome webstore](https://chrome.google.com/webstore/detail/postman/fhbjgbiflinjbdggehcddcbncdddomop).



## API Usage Examples
### Get index
```
GET http://w7500xRESTAPI.local
```
  - Results: It returns all resources list supported by the board as representation in JSON.
    - Includes defined id list for I/O pins
  - URL '/' is substituted with '/index', Thus, this URL is recognized as [http://w7500xRESTAPI.local/index]

<p align="center">
  <img width="70%" src="https://ericslabs.files.wordpress.com/2016/03/get_index-1.png" />
</p>

### Get netinfo
```
GET http://w7500xRESTAPI.local/netinfo
```
  - Results: It returns board's network setting like MAC / IP address as representation in JSON.

<p align="center">
  <img width="70%" src="https://ericslabs.files.wordpress.com/2016/03/get_netinfo_.png" />
</p>

### Get userio
```
GET http://w7500xRESTAPI.local/userio
```
  - Results: It returns setting of activated IO on board as representation in JSON. Each IO's JSON object includes three-keys. (ID / Type / Direction)

<p align="center">
  <img width="70%" src="https://ericslabs.files.wordpress.com/2016/03/get_userio.png" />
</p>
