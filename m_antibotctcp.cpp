/*
 * InspIRCd -- Internet Relay Chat Daemon
 *
 *   Copyright (C) Nikos `UrL` Papakonstantinou <url.euro@gmail.com>
 *
 * This file is part of InspIRCd.  InspIRCd is free software: you can
 * redistribute it and/or modify it under the terms of the GNU General Public
 * License as published by the Free Software Foundation, version 2.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "inspircd.h"

/* $ModConfig: <antifloodctcp link="http://yoursite.gr" sendsnotice="true" msgonreply="true"> */
/* $ModDesc: Blocks any type of floodbot/spambot. */
/* $ModAuthor: Nikos `UrL` Papakonstantinou */
/* $ModAuthorMail: url.euro@gmail.com */
/* $ModDepends: core 2.0-2.1 */

/*
 * Written by Nikos `UrL Papakonstantinou, 15 October 2014.
 * This module blocks any type of floodbot/spambot but it may kill
 * legitimate users with broken clients so use it at your own risk.
 * Thanks to Attila for various fixes.
 * Thanks to Ctcp for the tests.
 */

/* What it does
 * This module improves the network security against flood/spam scripts.
 * It works with RequireCTCPTime so any client without CTCP Time reply
 * is blocked.It doesn't apply any XLINE yet so the user can change the
 * client any time.
 * TODO: Short XLINES to improve the security & Webform for auto unblock.
 */

class ModuleAntiBotCTCP : public Module
{
	bool sendsnotice;
	bool msgonreply;
	LocalIntExt ext;
	std::string link;

 public:
	ModuleAntiBotCTCP()
		: ext("ctcptime_wait", this)
	{
	}

	void init()
	{
		ServerInstance->Modules->AddService(ext);
		Implementation eventlist[] = { I_OnUserRegister, I_OnPreCommand, I_OnCheckReady, I_OnRehash };
		ServerInstance->Modules->Attach(eventlist, this, sizeof(eventlist)/sizeof(Implementation));
		OnRehash(NULL);
	}

	Version GetVersion()
	{
		return Version("Blocks any type of floodbot/spambot.");
	}

	void OnRehash(User* user)
	{
		ConfigTag* tag = ServerInstance->Config->ConfValue("antibotctcp");
		sendsnotice = tag->getBool("sendsnotice", true);
		msgonreply = tag->getBool("msgonreply", true);
		link = tag->getString("link");
	}

	ModResult OnUserRegister(LocalUser* user)
	{
		user->WriteNumeric(931, "%s :Malicious or potentially unwanted softwares are not WELCOME here!", user->nick.c_str());
		user->WriteServ("PRIVMSG %s :\1TIME\1", user->nick.c_str());
		if (sendsnotice)
			user->WriteServ("NOTICE " + user->nick + " :*** If you are having problems connecting to this server, please get a better CLIENT or visit " + link + " for more info.");

		ext.set(user, 1);
		return MOD_RES_PASSTHRU;
	}

	ModResult OnPreCommand(std::string &command, std::vector<std::string> &parameters, LocalUser* user, bool validated, const std::string &original_line)
	{
		if (command == "NOTICE" && !validated && parameters.size() > 1 && ext.get(user))
		{
			// TODO: check if it's a CTCP TIME reply
			ext.set(user, 0);
			if (msgonreply)
				user->WriteServ("NOTICE " + user->nick + " :*** Howdy buddy,you are authorized to use this server!");
			return MOD_RES_DENY;
		}
		return MOD_RES_PASSTHRU;
	}

	ModResult OnCheckReady(LocalUser* user)
	{
		if (ext.get(user))
			return MOD_RES_DENY;
		return MOD_RES_PASSTHRU;
	}
};

MODULE_INIT(ModuleAntiBotCTCP)
