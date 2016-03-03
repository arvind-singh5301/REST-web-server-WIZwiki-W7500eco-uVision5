# RESTful Web Server & I/O Control by REST API
Simple REST Web server library for small IoT devices. 
Users can be made the IoT device for REST-based web services available in this project using C language / WIZwiki-W7500ECO platform board.

## Target Board and IDE
### WIZWiki-W7500ECO
<!-- WIZwiki-W7500ECO pic -->
<p align="center">
  <img width="60%" src="http://wizwiki.net/wiki/lib/exe/fetch.php?cache=&media=products:wizwiki-w7500eco:wizwiki-w7500eco3dtop.png" />
</p>

<!-- WIZwiki-W7500ECO pinout -->
<p align="center">
  <img width="80%" src="http://wizwiki.net/wiki/lib/exe/fetch.php?media=products:wizwiki-w7500eco:wizwiki-w7500eco_detailpinout.png" />
</p>

For more details, please refer to [WIZ550web Wiki page](http://wizwiki.net/wiki/doku.php?id=products:wizwiki-w7500eco:start) in [WIZnet Wiki](http://wizwiki.net).

### Keil uVision5


## REST API Design

* REST API Document is under construction.

[:id] : pre-defined 4-I/O pins, 'a', 'b', 'c', 'd'.
  - 'a' : P30 pin (digital input / digital output / analog input)
  - 'b' : P29 pin (digital input / digital output / analog input)
  - 'c' : P28 pin (digital input / digital output / analog input)
  - 'd' : P27 pin (digital input / digital output / analog input)

### HTTP GET method
  - http://w7500xRESTAPI.local/index
  - http://w7500xRESTAPI.local/uptime
  - http://w7500xRESTAPI.local/netinfo
  - http://w7500xRESTAPI.local/userio
  - http://w7500xRESTAPI.local/userio/:id
  - http://w7500xRESTAPI.local/userio/:id/info

### HTTP POST method
  - http://w7500xRESTAPI.local/userio/:id

### HTTP PUT method* (in development)
  - http://w7500xRESTAPI.local/userio/:id
  - http://w7500xRESTAPI.local/userio/:id/info

### HTTP DELETE method
  - http://w7500xRESTAPI.local/userio/:id



## Testing
Connect your board to your network and run test tool. These library has been tested on Postman Builder.

For more details about Postman, please refer to [Postman Chrome webstore](https://chrome.google.com/webstore/detail/postman/fhbjgbiflinjbdggehcddcbncdddomop).



## Usage
### Under construction
