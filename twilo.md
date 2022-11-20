## Twilo

```
curl 'https://api.twilio.com/2010-04-01/Accounts/AC35f55d787a8a836461c3c932ca9ce61d/Messages.json' -X POST \
--data-urlencode 'To=whatsapp:+50370398239' \
--data-urlencode 'From=whatsapp:+14155238886' \
--data-urlencode 'Body=Hello this is a test' \
-u AC35f55d787a8a836461c3c932ca9ce61d:5b6fcd2abc02fa9ec8522f352f173c31
```