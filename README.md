m_antispambot.cpp
=======

* ModDesc: Blocks any type of floodbot/spambot. 
* ModAuthor: Nikos `UrL` Papakonstantinou
* ModAuthorMail: url.euro@gmail.com 
* ModDepends: core 2.0-2.1 
* ModVersion: $Rev: 22 $ 
* Thanks to Attila for various fixes.
* Thanks to Ctcp for the tests.

Written by Nikos `UrL Papakonstantinou, 15 Octomber 2014. 
Originally based on m_antibear.

Desc
=======
This module blocks any type of floodbot/spambot but it may kill 
legitimate users with broken clients so use it at your own risk.


Installation
=======
* 1. Compile the module 
* 2. Add the following lines at your modules.conf 
  `<module name="m_antispambot.so">` and
  `<AntiSpambot link="http://yoursite.gr" sendsnotice="false" msgonreply="true">`
* 3. Rehash
