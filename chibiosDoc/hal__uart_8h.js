var hal__uart_8h =
[
    [ "UART_NO_ERROR", "group___u_a_r_t.html#ga0cdcd213b2d6910d7228ae2564679486", null ],
    [ "UART_PARITY_ERROR", "group___u_a_r_t.html#ga946e3d317937e003d2057bf19e96dd1d", null ],
    [ "UART_FRAMING_ERROR", "group___u_a_r_t.html#ga1d37ef27139eda65ba7e8da0fbf4c1cc", null ],
    [ "UART_OVERRUN_ERROR", "group___u_a_r_t.html#ga3183177e3613d8785d8cc8516931beb6", null ],
    [ "UART_NOISE_ERROR", "group___u_a_r_t.html#ga89cdd2b33815a8f6464b18bb9748ae95", null ],
    [ "UART_BREAK_DETECTED", "group___u_a_r_t.html#ga45bad0a1296763c304a19c982315657a", null ],
    [ "UART_USE_WAIT", "group___u_a_r_t.html#ga3ccbb19b26b53ba0a8e40cb6b8650114", null ],
    [ "UART_USE_MUTUAL_EXCLUSION", "group___u_a_r_t.html#gae689e57cf792af401c324de566038687", null ],
    [ "_uart_wakeup_tx1_isr", "group___u_a_r_t.html#gaa36d3b29ef0578de7695beacc1b817c1", null ],
    [ "_uart_wakeup_tx2_isr", "group___u_a_r_t.html#ga05460d44b028d7dc38594da44b41e8e5", null ],
    [ "_uart_wakeup_rx_complete_isr", "group___u_a_r_t.html#gad749245f0bed7d023e7b066740f96f6e", null ],
    [ "_uart_wakeup_rx_error_isr", "group___u_a_r_t.html#ga637ade05063ffd114773945d871ef5ec", null ],
    [ "_uart_wakeup_rx_timeout_isr", "group___u_a_r_t.html#ga40eebebe32bd2f988f314a8c7554fa49", null ],
    [ "_uart_tx1_isr_code", "group___u_a_r_t.html#ga54586c211569cfe86ba2f197dd7a315b", null ],
    [ "_uart_tx2_isr_code", "group___u_a_r_t.html#ga2e2655d7724b92527bab1c18a4da939e", null ],
    [ "_uart_rx_complete_isr_code", "group___u_a_r_t.html#ga541e5240a92b477a8983caf66cc19743", null ],
    [ "_uart_rx_error_isr_code", "group___u_a_r_t.html#ga3cfad83951b3bd15efc63c745881293c", null ],
    [ "_uart_rx_idle_code", "group___u_a_r_t.html#ga73f91a0f01d448cbadcd0af3d181269e", null ],
    [ "_uart_timeout_isr_code", "group___u_a_r_t.html#ga919cb4ff4826f3559af89568ec762eed", null ],
    [ "uartstate_t", "group___u_a_r_t.html#ga554a8b398f6e066b0b3fb0ff4f304845", [
      [ "UART_UNINIT", "group___u_a_r_t.html#gga554a8b398f6e066b0b3fb0ff4f304845a93a7f6d021e140d9211b31668af3b899", null ],
      [ "UART_STOP", "group___u_a_r_t.html#gga554a8b398f6e066b0b3fb0ff4f304845ada933bf8409b243459b1600e68c250cc", null ],
      [ "UART_READY", "group___u_a_r_t.html#gga554a8b398f6e066b0b3fb0ff4f304845a204185132a3b6bc97b53c3529d81dcec", null ]
    ] ],
    [ "uarttxstate_t", "group___u_a_r_t.html#ga6022979e2b7b8edd09b5396237695b1b", [
      [ "UART_TX_IDLE", "group___u_a_r_t.html#gga6022979e2b7b8edd09b5396237695b1ba7b938ac5e01cf34d8d0dffb788980180", null ],
      [ "UART_TX_ACTIVE", "group___u_a_r_t.html#gga6022979e2b7b8edd09b5396237695b1bac1bec1596509c7e04913124de024f21c", null ],
      [ "UART_TX_COMPLETE", "group___u_a_r_t.html#gga6022979e2b7b8edd09b5396237695b1bad0138ae2b7d3814fa3462f28e8489591", null ]
    ] ],
    [ "uartrxstate_t", "group___u_a_r_t.html#ga590357d9275f2281ec439d8a5bbdfd82", [
      [ "UART_RX_IDLE", "group___u_a_r_t.html#gga590357d9275f2281ec439d8a5bbdfd82a3582379c6e7ca546503aebfdbfcd1fdc", null ],
      [ "UART_RX_ACTIVE", "group___u_a_r_t.html#gga590357d9275f2281ec439d8a5bbdfd82a118883b969326bb7cad7bff02b826f4c", null ],
      [ "UART_RX_COMPLETE", "group___u_a_r_t.html#gga590357d9275f2281ec439d8a5bbdfd82ae9eec7764042faeb4caf0471fc40fddb", null ]
    ] ],
    [ "uartInit", "group___u_a_r_t.html#ga09a7a9ed4194733a7b753c3b795b1734", null ],
    [ "uartObjectInit", "group___u_a_r_t.html#ga2a4d715584d673eb2da4c2f201edd0db", null ],
    [ "uartStart", "group___u_a_r_t.html#ga82eb06062e2fa1ab8b5c53576724bb50", null ],
    [ "uartStop", "group___u_a_r_t.html#gaa42e8f4567939942a15ea2543e84471a", null ],
    [ "uartStartSend", "group___u_a_r_t.html#ga16ef747df9b18390c05c9401cd04f626", null ],
    [ "uartStartSendI", "group___u_a_r_t.html#ga2156bcae3b85d6e86a21a494cd0a7d42", null ],
    [ "uartStopSend", "group___u_a_r_t.html#gae6e8a633c331dd84be85d2bec8ab9008", null ],
    [ "uartStopSendI", "group___u_a_r_t.html#gaedd13992b5f0fe148c77dc88c5b2e5c2", null ],
    [ "uartStartReceive", "group___u_a_r_t.html#gae3a748783e536dbebc87748b95a78ab2", null ],
    [ "uartStartReceiveI", "group___u_a_r_t.html#ga957c942d05cdbcc4daf00c3e34b20d48", null ],
    [ "uartStopReceive", "group___u_a_r_t.html#gaed44127f7ad3e6b34df467f4f43b69fd", null ],
    [ "uartStopReceiveI", "group___u_a_r_t.html#gaee09f7e46f4ee4eb212440a33ffc82f1", null ],
    [ "uartSendTimeout", "group___u_a_r_t.html#gaf1a5a90aa9ce64a515ba6f88b06c8841", null ],
    [ "uartSendFullTimeout", "group___u_a_r_t.html#ga9f8a475dcef00e0a42e2ebb10cffae8b", null ],
    [ "uartReceiveTimeout", "group___u_a_r_t.html#ga9e0a47ed3122f1bfad354601e70cd913", null ],
    [ "uartAcquireBus", "group___u_a_r_t.html#ga7991547d40265b6c9766b6c9977153df", null ],
    [ "uartReleaseBus", "group___u_a_r_t.html#ga9eb1a374d69b742e9c0bd0b723ceac5f", null ]
];