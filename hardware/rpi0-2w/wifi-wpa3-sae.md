# Не подключается Wi-Fi WPA2/WPA3: кейс SAE

## Содержание

- [Проблема](#проблема)
- [Применённое решение](#применённое-решение)
- [Результат](#результат)

## Проблема

На Raspberry Pi Zero 2 W не получалось подключиться к сети `UL` с WPA2/WPA3. Подключение зависало на аутентификации SAE (WPA3), в журнале появлялись `status_code=16` и тайм-ауты.

Пароль был правильным: когда принудительно выбрали WPA2, подключение прошло. Настройка NetworkManager `wpa-psk` сама по себе не исключала SAE — она допускает WPA2 и WPA3 Personal, как указано в [документации](https://networkmanager.dev/docs/api/latest/settings-802-11-wireless-security.html).

## Применённое решение

Ниже — обход, применённый 7 октября 2026 года на BCM43430 с драйвером `brcmfmac` и ядром `6.18.50+rpt-rpi-v8`. Это настройка драйвера всей платы; её применимость к другим версиям нужно проверять отдельно.

Команды выполняются **на Raspberry Pi через UART-консоль**, поскольку Wi-Fi будет отключён. Профиль `UL` уже создан и содержит пароль.

1. Создали `/etc/modprobe.d/ul-wpa2.conf` со следующим полным содержимым:

```conf
options brcmfmac feature_disable=0x2282000
```

Создание файла:

```bash
printf '%s\n' 'options brcmfmac feature_disable=0x2282000' | sudo tee /etc/modprobe.d/ul-wpa2.conf
```

2. Отключили соединение, перезагрузили модули драйвера и подключились заново:

```bash
sudo nmcli connection modify 'UL' connection.autoconnect no
sudo nmcli connection down 'UL'
sudo modprobe -r brcmfmac_cyw brcmfmac
sudo modprobe brcmfmac
sudo nmcli --wait 30 connection up 'UL'
sudo nmcli connection modify 'UL' connection.autoconnect yes
```

3. Обновили initramfs для ядра исходной системы и проверили включение файла:

```bash
sudo update-initramfs -u -k 6.18.50+rpt-rpi-v8
lsinitramfs /boot/initrd.img-6.18.50+rpt-rpi-v8 | grep ul-wpa2
```

Маска отключает SAE и несколько других функций драйвера; это применённый обход, а не доказанное исправление первопричины.

## Результат

После переподключения проверили:

```bash
sudo wpa_cli -i wlan0 status
ping -c 3 1.1.1.1
getent hosts raspberrypi.com
```

Получили `wpa_state=COMPLETED`, `key_mgmt=WPA2-PSK` и IP-адрес. Ping и DNS работали. То есть через WPA2 плата подключилась к той же сети с тем же паролем.

Точная причина отказа SAE не установлена. Полную перезагрузку платы тогда не проверяли; сохранение результата после reboot остаётся отдельной проверкой.

К вопросу вернёмся отдельно: временное подключение через `wpa_cli` с `WPA-PSK` работало ещё до создания `ul-wpa2.conf`. Нужно проверить, можно ли обеспечить повторное подключение и работу после reboot без этой настройки драйвера.

[К описанию rpi0-2w](README.md)
