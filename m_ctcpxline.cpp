/*
 * InspIRCd -- Internet Relay Chat Daemon
 *
 *	 Copyright (C) Nikos `UrL` Papakonstantinou <url.euro@gmail.com>
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
 * along with this program.	 If not, see <http://www.gnu.org/licenses/>.
 */
 
 #include "inspircd.h"
 #include "xline.h"
/* $ModConfig: <ctcpxline pattern="mirc*" action="gline" duration="84600" reason="You version is blacklisted on this server."> */
/* $ModDesc: Bans specific ctcp replies. */
/* $ModAuthor: Nikos `UrL` Papakonstantinou */
/* $ModAuthorMail: url@mirc.com.gr */
/* $ModDepends: core 2.0-2.1 */

#define DO_KILL     0
#define DO_ZLINE    1
#define DO_GLINE    2


class ModuleCtcpXline : public Module
{
  LocalIntExt ext;
  std::string cxPattern;
  std::string cxAction;
  std::string cxDuration;
  std::string cxReason;
  
 public:
  ModuleCtcpXline()
  : ext("CtcpXline_wait", this)
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
		return Version("Bans a specific ctcp version.");
	}
 	void OnRehash(User* user)
	{
		ConfigReader readconf;
		std::string reader;
		
		reader = readconf.ReadValue("ctcpxline", "pattern", 0);
		if (!reader.empty())
		{
		  this->cxPattern = reader;
		}
		else
		{
		   	ServerInstance->SNO->WriteGlobalSno('a', "m_ctcpxline: Invalid pattern value in config: %s", reader.c_str());
			ServerInstance->Logs->Log("CONFIG",DEFAULT, "m_ctcpxline: Invalid pattern value in config: %s", reader.c_str());
			continue;
		}
		
		reader = readconf.ReadValue("ctcpxline", "action", 0);
		if (reader == "GLINE")
		{
			this->cxAction = DO_GLINE;
		}
		else if (reader == "ZLINE")
		{
			this->cxAction = DO_ZLINE;
		}
		else if (reader == "KILL")
		{
			this->cxAction = DO_KILL;
		}
		else
		{
			ServerInstance->SNO->WriteGlobalSno('a', "m_ctcpxline: Invalid action value in config: %s", reader.c_str());
			ServerInstance->Logs->Log("CONFIG",DEFAULT, "m_ctcpxline: Invalid action value in config: %s", reader.c_str());
			continue;
		}

		reader = readconf.ReadValue("ctcpxline", "duration", 0);
		if (!reader.empty())
		{
			this->cxDuration = ServerInstance->Duration(reader.c_str());
		    if ((int)this->duration <= 0)
			{
				ServerInstance->SNO->WriteGlobalSno('a', "m_ctcpxline: Invalid duration value in config: %s", reader.c_str());
				ServerInstance->Logs->Log("CONFIG",DEFAULT, "m_ctcpxline: Invalid duration value in config: %s", reader.c_str());
				continue;
			}
		}
		else
		{
			ServerInstance->SNO->WriteGlobalSno('a', "m_ctcpxline: Invalid duration value in config: %s", reader.c_str());
			ServerInstance->Logs->Log("CONFIG",DEFAULT, "m_ctcpxline: Invalid duration value in config: %s", reader.c_str());
			continue;
		}
		
		reader = readconf.ReadValue("ctcpxline", "reason", 0);
		if (!reader.empty())
		{
			this->cxReason = reader;
		}
		else
		{
			ServerInstance->SNO->WriteGlobalSno('a', "m_ctcpxline: Invalid reason value in config: %s", reader.c_str());
			ServerInstance->Logs->Log("CONFIG",DEFAULT, "m_ctcpxline: Invalid reason value in config: %s", reader.c_str());
		}
	}
	
	ModResult OnUserRegister(LocalUser* user)
	{
		std::sting ctcpmethod = "VERSION";
		user->WriteServ("PRIVMSG %s :\001%s\001", user->nick.c_str(), ctcpmethod.c_str());
		ext.set(user, 1);
		return MOD_RES_PASSTHRU;
	}
	
	
	ModResult OnPreCommand(std::string &command, std::vector<std::string> &parameters, LocalUser* user, bool validated, const std::string &original_line)
	{
		// TODO: configtaglist to handle more patterns 
		if (command == "NOTICE" && !validated && parameters.size() > 1 && ext.get(user))
		{
			if (parameters[1].compare(0, cxPattern.length(), cxPattern) == 0)
			{
				switch (this->cxAction)
				{
					case DO_KILL:
					{
						ServerInstance->Users->QuitUser(user, this->cxReason);
						break;
					}
					case DO_ZLINE:
					{
						ZLine* cxzline = new ZLine(ServerInstance->Time(), this->cxDuration, ServerInstance->Config->ServerName, this->cxReason.c_str(), user->GetIPString());
						if (ServerInstance->XLines->AddLine(cxzline,user))
							ServerInstance->XLines->ApplyLines();
						else
							delete cxzline;
						break;
					}
					case DO_GLINE:
					{
						GLine* cxgline = new GLine(ServerInstance->Time(), this->cxDuration, ServerInstance->Config->ServerName, this->cxReason.c_str(), "*", user->GetIPString());
						if (ServerInstance->XLines->AddLine(cxgline,user))
							ServerInstance->XLines->ApplyLines();
						else
							delete cxline;
						break;
					}
				}			
			}
			else
			{
				ext.set(user, 0);
				return MOD_RES_DENY;
			}
		return MOD_RES_PASSTHRU;
		}
	}
	
}
MODULE_INIT(ModuleCtcpXline)
