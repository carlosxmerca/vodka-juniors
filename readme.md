## Vodja Juniors
### Sistema de riego automatizado para interiores con función de notificaciones

#### Descripción del proyecto
Un circuito con el objetivo de administrar el suministro de agua a una planta, controlado por medio del ESP8266, almacenará información utilizando Adafruit y utilizará notificaciones por Whatsapp. La población que se pretende beneficiar son todos los entusiastas de la jardinería y personas que quieren mantener sus plantas bonitas sin estar muy pendientes de estas. 

El proyecto busca aprovechar el agua reciclada (como el agua lluvia) y al mismo tiempo recuperar información de cómo se comporta la lectura de humedad en el entorno de la planta en cuestión al respetar sus necesidades hídricas, con esto se pueden identificar patrones o variaciones en las necesidades hídricas la planta en diferentes épocas del año.

El circuito consta de un sensor ultrasónico (HC-SR04) encargado de medir la cantidad de agua contenida dentro del recipiente (la cual será utilizada para el riego), con el objetivo de notificar al usuario cuando:
- Esté lleno
- Esté a capacidad media
- Esté en nivel bajo
- El agua se ha terminado

Se programara una rutina semanal de los momentos clave de riego y la cantidad de agua a utilizar, para esto se utilizara una bomba de agua pequeña controlada por medio del ESP8266. De igual manera se notificará al usuario por Whatsapp cada vez que la planta sea regada.

Adicionalmente el sensor de humedad en el suelo (FC-28) y el sensor de humedad y temperatura (DHT-11) proporcionarán datos que serán almacenados en Adafruit para su posterior estudio, lo que se busca encontrar son patrones y establecer modelos de correlación entre el agua suministrada, las lecturas de humedad y la temperatura ambiente.
