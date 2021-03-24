# 2021-03-24_102134
:docker:mqtt:mosquitto:no_papel:

## Configuration of Mosquitto v2+ user authentication in Docker

Eclipse's Mosquitto MQTT broker by default allows anonymous publish and
subscription operations to the broker. In order reject anonymous connections,
user authentication must be configured. 

**NOTE**: user authentication in Eclipse's Mosquitto does not encrypt the
contents of the files, thus, all communications with the broker travel in
plain-text.

Mosquitto manages this by setting up a special password file, typically
`mosquitto.passwd`, that is employed to keep a list of all the user logins.

Without Docker, this password configuration is done via the `mosquitto_passwd`
tool, but since we need to use these credentials in a container, additional
steps are required.

Additionally, the path to this file must be specified in the configuration file
`mosquitto.conf`.

In order to manage this configuration with docker, both `mosquitto.conf` and
`mosquitto.passwd` must be created locally and mounted in the container.

First, lest begin by creating the configuration files in our workdir, where the
`docker-compose.yml` file is.


```
cd workdir # this is where the docker-compose.yml file is located

# create the config and password files in case they did not exist before
mkdir -p ./configuration/eclipse-mosquitto
touch ./configuration/eclipse-mosquitto/mosquitto.conf
touch ./configuration/eclipse-mosquitto/mosquitto.passwd
```

The eclipse-mosquitto docker container uses the configuration file 
located at `/mosquitto/config/mosquitto.conf`, and the password file is
specified in that same configuration file. For consistency, we will place both in the same directory.

Hence, the password file will be placed in `/mosquitto/config/mosquitto.passwd`
inside the docker container.

### Running as Docker container

```
cd workdir # this is where the docker-compose.yml file is located
docker run -d -p 1883:1883 -v "$(pwd)/configuration/eclipse-mosquitto/mosquitto.conf:/mosquitto/config/mosquitto.conf" -v "$(pwd)/configuration/eclipse-mosquitto/mosquitto.passwd:/mosquitto/config/mosquitto.passwd" eclipse-mosquitto:2
```

Where `$(pwd)/configuration/eclipse-mosquitto/` is the local absolute path to your configuration files in the host.

### Running as Docker Compose

Alternatively you can run it with Docker Compose, the `docker-compose.yml` should similar to the following example:

```
version: "3"
services:

# [...Other container's configurations...]

 mosquitto:
    image: eclipse-mosquitto:2
    ports:
      - 1883:1883
    volumes:
      - ./configuration/eclipse-mosquitto/mosquitto.conf:/mosquitto/config/mosquitto.conf
      - ./configuration/eclipse-mosquitto/mosquitto.passwd:/mosquitto/config/mosquitto.passwd
		
		
# [...Other container's configurations...]
```
To run the containers

```
cd path_to_the_docker_compose_file # this is where the docker-compose.yml file is located
docker-compose up -d
```

To stop the containers

```
docker-compose down
```


### Configuration of password files

Edit the  host's `./configuration/eclipse-mosquitto/mosquitto.conf`:

```
listener 1883
allow_anonymous false
password_file /mosquitto/config/mosquitto.passwd
```

**Note** that this configuration file disables the subscription and publication
of messages to this broker without user and password login. This is due to the
`allow_anonymous false` statement. But at this moment, we have not configured a
user login yet.

Now, we need to establish the user credentials in the `mosquitto.passwd`. This can be done both ways:

- Inside the mosquitto docker container itself.
- From the host, considering that it has the `mosquitto_passwd` tool installed locally.

**Note**: for compatibility reasons, it is suggested to use the
`mosquitto_passwd` tool inside the mosquitto docker container itself instead of
the one installed locally in the host, since the versions could differ and generate
incompatible results.

To edit the `mosquitto.passwd` file from within the docker container, open an terminal within the container:

```
docker exec --interactive --tty name_of_the_eclipse_mosquito_container /bin/sh
```

Next, configure the 

```
# At this point commands are run within the mosquitto container
cd /mosquitto/config/
mosquitto_passwd -c /mosquitto/config/mosquitto.passwd username
```

Where `username` is the desired login name for the credentials.

**NOTE** that `mosquitto_passwd -c` creates a new password file, overriding
previously created files. Ommit the `-c` option if the password file already
exists.


At this point we're asked for the password twice. Once finished, we can see the results in the file.

```
cat mosquitto.passwd
```
We should see something similar to this:

```
username:$7$101$zWb13KiZwN/664Jo$C23QLyIF7ANzAZuvdbWBPAFQ/qlwX8zrEevbtLwPcavs1cEnPx9w+9hONwx/s5cNrC8b/KoBImXvgPgve9ospw==
```

We can exit the container now with `exit`.

Restart the container for the changes to take effect.

From now on, to subscribe or publish to the container, we require a user and
password for the `mosquitto_pub` and `mosquitto_sub` tools.

### Configuration of Access Control Lists

Similar to the configuration of user authentication in the previous sections,
mosquitto allows the configuration of access control lists.  In order to do so,
we require another file specifying the access control directives, e.g.,
`mosquitto.acl`. This file constrains the access of different users to topics.

- `mosquitto.conf` example:

```
allow_anonymous false
acl_file /mosquitto/config/mosquitto.acl
password_file /mosquitto/config/mosquitto.passwd
```

- `mosquitto.acl` file example

```
user user1
topic /topics1/#

user user2
topic /topics2/+/#
```

### TODO

Review and document the following configuration options:

```

# Place your local configuration in /etc/mosquitto/conf.d/
#
# A full description of the configuration file is at
# /usr/share/doc/mosquitto/examples/mosquitto.conf.example

pid_file /var/run/mosquitto.pid

persistence true
# WARNING the following line MUST end with "/"
persistence_location /var/lib/mosquitto/

log_dest file /var/log/mosquitto/mosquitto.log

include_dir /etc/mosquitto/conf.d

```

```

autosave_interval 7200
max_inflight_messages 10
allow_zero_length_clientid false

allow_anonymous false
acl_file /etc/mosquitto/aclfile
password_file /etc/mosquitto/passwd

connection_messages true

```
