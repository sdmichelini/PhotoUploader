<h1>Photo Uploader</h1>

<h2>Program Features</h2>

-Written in C++

-Simulates TCP Data Link Layer and Physical Layer with Artificial Error Injection

-Transfers JPEGs

<h3>Server</h3>

<h4>Application Layer</h4>

-Concurrent with multiple processes

-Accepts a Remote Connection and Attempts to Read a JPEG image

<h4>Data-Link Layer</h4>

-Turns received frames into packets and send them to application layer

-Use’s 16-bit XOR folding method to do error-detection

```C++
uint16_t data_link_layer::xorFold(char * array, unsigned long size){
	uint16_t result = 0;
	//We are doing a two-byte check so we only go to size/2 and read at 2i
	for(unsigned int i = 0; i < (size/2); i++){
		uint16_t curNum = 0;

		//Copy in the current packet
		memcpy(&curNum, (char *)(array + 2*i), sizeof(uint16_t));

		//XOR fold
		result ^= curNum;
	}
	return result;
}
```

<h4>Physical Layer</h4>

-Implements Actual TCP calls

-Timeout implemented with select() call and a timeout

<h3>Client</h3>

<strong>Usage:</strong> ./concurrent_photo_client (server to connect to) (client id number) (number of photos to upload)

-Read’s in a user-specified number of photos and tries to upload them to server