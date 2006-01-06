/*
 * commands.c     -- user commands handling
 *
 * Copyright (C) 2005 Mikael Berthe <bmikael@lists.lilotux.net>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or (at
 * your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
 * USA
 */

#include <string.h>

#include "commands.h"
#include "jabglue.h"
#include "roster.h"
#include "screen.h"
#include "compl.h"
#include "hooks.h"
#include "hbuf.h"
#include "utils.h"
#include "settings.h"

// Commands callbacks
static void do_roster(char *arg);
static void do_status(char *arg);
static void do_status_to(char *arg);
static void do_add(char *arg);
static void do_del(char *arg);
static void do_group(char *arg);
static void do_say(char *arg);
static void do_msay(char *arg);
static void do_say_to(char *arg);
static void do_buffer(char *arg);
static void do_clear(char *arg);
static void do_info(char *arg);
static void do_rename(char *arg);
static void do_move(char *arg);
static void do_set(char *arg);
static void do_alias(char *arg);
static void do_bind(char *arg);
static void do_connect(char *arg);
static void do_disconnect(char *arg);
static void do_rawxml(char *arg);
static void do_room(char *arg);
static void do_authorization(char *arg);
static void do_version(char *arg);

// Global variable for the commands list
static GSList *Commands;


//  cmd_add()
// Adds a command to the commands list and to the CMD completion list
void cmd_add(const char *name, const char *help,
        guint flags_row1, guint flags_row2, void (*f)())
{
  cmd *n_cmd = g_new0(cmd, 1);
  strncpy(n_cmd->name, name, 32-1);
  n_cmd->help = help;
  n_cmd->completion_flags[0] = flags_row1;
  n_cmd->completion_flags[1] = flags_row2;
  n_cmd->func = f;
  Commands = g_slist_append(Commands, n_cmd);
  // Add to completion CMD category
  compl_add_category_word(COMPL_CMD, name);
}

//  cmd_init()
// ...
void cmd_init(void)
{
  cmd_add("add", "Add a jabber user", COMPL_JID, 0, &do_add);
  cmd_add("alias", "Add an alias", 0, 0, &do_alias);
  cmd_add("authorization", "Manage subscription authorizations",
          COMPL_AUTH, COMPL_JID, &do_authorization);
  cmd_add("bind", "Add an key binding", 0, 0, &do_bind);
  cmd_add("buffer", "Manipulate current buddy's buffer (chat window)",
          COMPL_BUFFER, 0, &do_buffer);
  cmd_add("clear", "Clear the dialog window", 0, 0, &do_clear);
  cmd_add("connect", "Connect to the server", 0, 0, &do_connect);
  cmd_add("del", "Delete the current buddy", 0, 0, &do_del);
  cmd_add("disconnect", "Disconnect from server", 0, 0, &do_disconnect);
  cmd_add("group", "Change group display settings", COMPL_GROUP, 0, &do_group);
  //cmd_add("help", "Display some help", COMPL_CMD, 0, NULL);
  cmd_add("info", "Show basic infos on current buddy", 0, 0, &do_info);
  cmd_add("move", "Move the current buddy to another group", COMPL_GROUPNAME,
          0, &do_move);
  cmd_add("msay", "Send a multi-lines message to the selected buddy",
          COMPL_MULTILINE, 0, &do_msay);
  cmd_add("room", "MUC actions command", COMPL_ROOM, 0, &do_room);
  cmd_add("quit", "Exit the software", 0, 0, NULL);
  cmd_add("rawxml", "Send a raw XML string", 0, 0, &do_rawxml);
  cmd_add("rename", "Rename the current buddy", 0, 0, &do_rename);
  cmd_add("roster", "Manipulate the roster/buddylist", COMPL_ROSTER, 0,
          &do_roster);
  cmd_add("say", "Say something to the selected buddy", 0, 0, &do_say);
  cmd_add("say_to", "Say something to a specific buddy", COMPL_JID, 0,
          &do_say_to);
  //cmd_add("search");
  cmd_add("set", "Set/query an option value", 0, 0, &do_set);
  cmd_add("status", "Show or set your status", COMPL_STATUS, 0, &do_status);
  cmd_add("status_to", "Show or set your status for one recipient",
          COMPL_JID, COMPL_STATUS, &do_status_to);
  cmd_add("version", "Show mcabber version", 0, 0, &do_version);

  // Status category
  compl_add_category_word(COMPL_STATUS, "online");
  compl_add_category_word(COMPL_STATUS, "avail");
  compl_add_category_word(COMPL_STATUS, "invisible");
  compl_add_category_word(COMPL_STATUS, "free");
  compl_add_category_word(COMPL_STATUS, "dnd");
  compl_add_category_word(COMPL_STATUS, "notavail");
  compl_add_category_word(COMPL_STATUS, "away");

  // Roster category
  compl_add_category_word(COMPL_ROSTER, "bottom");
  compl_add_category_word(COMPL_ROSTER, "top");
  compl_add_category_word(COMPL_ROSTER, "up");
  compl_add_category_word(COMPL_ROSTER, "down");
  compl_add_category_word(COMPL_ROSTER, "hide_offline");
  compl_add_category_word(COMPL_ROSTER, "show_offline");
  compl_add_category_word(COMPL_ROSTER, "toggle_offline");
  compl_add_category_word(COMPL_ROSTER, "alternate");
  compl_add_category_word(COMPL_ROSTER, "search");
  compl_add_category_word(COMPL_ROSTER, "unread_first");
  compl_add_category_word(COMPL_ROSTER, "unread_next");

  // Roster category
  compl_add_category_word(COMPL_BUFFER, "clear");
  compl_add_category_word(COMPL_BUFFER, "bottom");
  compl_add_category_word(COMPL_BUFFER, "top");
  compl_add_category_word(COMPL_BUFFER, "up");
  compl_add_category_word(COMPL_BUFFER, "down");
  compl_add_category_word(COMPL_BUFFER, "search_backward");
  compl_add_category_word(COMPL_BUFFER, "search_forward");
  compl_add_category_word(COMPL_BUFFER, "date");
  compl_add_category_word(COMPL_BUFFER, "%");

  // Group category
  compl_add_category_word(COMPL_GROUP, "fold");
  compl_add_category_word(COMPL_GROUP, "unfold");
  compl_add_category_word(COMPL_GROUP, "toggle");

  // Multi-line (msay) category
  compl_add_category_word(COMPL_MULTILINE, "abort");
  compl_add_category_word(COMPL_MULTILINE, "begin");
  compl_add_category_word(COMPL_MULTILINE, "send");
  compl_add_category_word(COMPL_MULTILINE, "send_to");
  compl_add_category_word(COMPL_MULTILINE, "verbatim");

  // Room category
  compl_add_category_word(COMPL_ROOM, "affil");
  compl_add_category_word(COMPL_ROOM, "ban");
  compl_add_category_word(COMPL_ROOM, "destroy");
  compl_add_category_word(COMPL_ROOM, "invite");
  compl_add_category_word(COMPL_ROOM, "join");
  compl_add_category_word(COMPL_ROOM, "kick");
  compl_add_category_word(COMPL_ROOM, "leave");
  compl_add_category_word(COMPL_ROOM, "names");
  compl_add_category_word(COMPL_ROOM, "nick");
  compl_add_category_word(COMPL_ROOM, "privmsg");
  compl_add_category_word(COMPL_ROOM, "remove");
  compl_add_category_word(COMPL_ROOM, "role");
  compl_add_category_word(COMPL_ROOM, "topic");
  compl_add_category_word(COMPL_ROOM, "unlock");
  compl_add_category_word(COMPL_ROOM, "whois");

  // Authorization category
  compl_add_category_word(COMPL_AUTH, "allow");
  compl_add_category_word(COMPL_AUTH, "cancel");
  compl_add_category_word(COMPL_AUTH, "request");
}

//  expandalias(line)
// If there is one, expand the alias in line and returns a new allocated line
// If no alias is found, returns line
// Note : if the returned pointer is different from line, the caller should
//        g_free() the pointer after use
char *expandalias(char *line)
{
  const char *p1, *p2;
  char *word;
  const gchar *value;
  char *newline = line;

  // Ignore leading '/'
  for (p1 = line ; *p1 == '/' ; p1++)
    ;
  // Locate the end of the word
  for (p2 = p1 ; *p2 && (*p2 != ' ') ; p2++)
    ;
  // Extract the word
  word = g_strndup(p1, p2-p1);

  // Look for an alias in the list
  value = settings_get(SETTINGS_TYPE_ALIAS, (const char*)word);
  if (value) {
    // There is an alias to expand
    newline = g_new(char, strlen(value)+strlen(p2)+2);
    *newline = '/';
    strcpy(newline+1, value);
    strcat(newline, p2);
  }
  g_free(word);

  return newline;
}

//  cmd_get
// Finds command in the command list structure.
// Returns a pointer to the cmd entry, or NULL if command not found.
cmd *cmd_get(const char *command)
{
  const char *p1, *p2;
  char *com;
  GSList *sl_com;

  // Ignore leading '/'
  for (p1 = command ; *p1 == '/' ; p1++)
    ;
  // Locate the end of the command
  for (p2 = p1 ; *p2 && (*p2 != ' ') ; p2++)
    ;
  // Copy the clean command
  com = g_strndup(p1, p2-p1);

  // Look for command in the list
  for (sl_com=Commands; sl_com; sl_com = g_slist_next(sl_com)) {
    if (!strcasecmp(com, ((cmd*)sl_com->data)->name))
      break;
  }
  g_free(com);

  if (sl_com)       // Command has been found.
    return (cmd*)sl_com->data;
  return NULL;
}

//  send_message(msg)
// Write the message in the buddy's window and send the message on
// the network.
static void send_message(const char *msg)
{
  const char *jid;

  if (!jb_getonline()) {
    scr_LogPrint(LPRINT_NORMAL, "You are not connected");
    return;
  }

  if (!current_buddy) {
    scr_LogPrint(LPRINT_NORMAL, "No buddy currently selected.");
    return;
  }

  jid = CURRENT_JID;
  if (!jid) {
    scr_LogPrint(LPRINT_NORMAL, "No buddy currently selected.");
    return;
  }

  if (buddy_gettype(BUDDATA(current_buddy)) != ROSTER_TYPE_ROOM) {
    // local part (UI, logging, etc.)
    hk_message_out(jid, NULL, 0, msg);
  }

  // Network part
  jb_send_msg(jid, msg, buddy_gettype(BUDDATA(current_buddy)), NULL);
}

//  process_command(line)
// Process a command line.
// Return 255 if this is the /quit command, and 0 for the other commands.
int process_command(char *line)
{
  char *p;
  char *xpline;
  cmd *curcmd;

  // Remove trailing spaces:
  for (p=line ; *p ; p++)
    ;
  for (p-- ; p>line && (*p == ' ') ; p--)
    *p = 0;

  // We do alias expansion here
  if (scr_get_multimode() != 2)
    xpline = expandalias(line);
  else
    xpline = line; // No expansion in verbatim multi-line mode

  // Command "quit"?
  if ((!strncasecmp(xpline, "/quit", 5)) && (scr_get_multimode() != 2) )
    if (!xpline[5] || xpline[5] == ' ')
      return 255;

  // If verbatim multi-line mode, we check if another /msay command is typed
  if ((scr_get_multimode() == 2) && (strncasecmp(xpline, "/msay ", 6))) {
    // It isn't an /msay command
    scr_append_multiline(xpline);
    return 0;
  }

  // Commands handling
  curcmd = cmd_get(xpline);

  if (!curcmd) {
    scr_LogPrint(LPRINT_NORMAL, "Unrecognized command, sorry.");
    if (xpline != line) g_free(xpline);
    return 0;
  }
  if (!curcmd->func) {
    scr_LogPrint(LPRINT_NORMAL, "Not yet implemented, sorry.");
    if (xpline != line) g_free(xpline);
    return 0;
  }
  // Lets go to the command parameters
  for (p = xpline+1; *p && (*p != ' ') ; p++)
    ;
  // Skip spaces
  while (*p && (*p == ' '))
    p++;
  // Call command-specific function
  (*curcmd->func)(p);
  if (xpline != line) g_free(xpline);
  return 0;
}

//  process_line(line)
// Process a command/message line.
// If this isn't a command, this is a message and it is sent to the
// currently selected buddy.
// Return 255 if the line is the /quit command, or 0.
int process_line(char *line)
{
  if (!*line) { // User only pressed enter
    if (scr_get_multimode()) {
      scr_append_multiline("");
      return 0;
    }
    if (current_buddy) {
      // Enter chat mode
      scr_set_chatmode(TRUE);
      scr_ShowBuddyWindow();
    }
    return 0;
  }

  if (*line != '/') {
    // This isn't a command
    if (scr_get_multimode())
      scr_append_multiline(line);
    else
      do_say(line);
    return 0;
  }

  /* It is (probably) a command -- except for verbatim multi-line mode */
  return process_command(line);
}

/* Commands callback functions */
/* All these do_*() functions will be called with a "arg" parameter */
/* (with arg not null)                                              */

static void do_roster(char *arg)
{
  if (!strcasecmp(arg, "top")) {
    scr_RosterTop();
    update_roster = TRUE;
  } else if (!strcasecmp(arg, "bottom")) {
    scr_RosterBottom();
    update_roster = TRUE;
  } else if (!strcasecmp(arg, "hide_offline")) {
    buddylist_set_hide_offline_buddies(TRUE);
    if (current_buddy)
      buddylist_build();
    update_roster = TRUE;
  } else if (!strcasecmp(arg, "show_offline")) {
    buddylist_set_hide_offline_buddies(FALSE);
    buddylist_build();
    update_roster = TRUE;
  } else if (!strcasecmp(arg, "toggle_offline")) {
    buddylist_set_hide_offline_buddies(-1);
    buddylist_build();
    update_roster = TRUE;
  } else if (!strcasecmp(arg, "unread_first")) {
    scr_RosterUnreadMessage(0);
  } else if (!strcasecmp(arg, "unread_next")) {
    scr_RosterUnreadMessage(1);
  } else if (!strcasecmp(arg, "alternate")) {
    scr_RosterJumpAlternate();
  } else if (!strncasecmp(arg, "search", 6)) {
    char *string = arg+6;
    if (*string && (*string != ' ')) {
      scr_LogPrint(LPRINT_NORMAL, "Unrecognized parameter!");
      return;
    }
    while (*string == ' ')
      string++;
    if (!*string) {
      scr_LogPrint(LPRINT_NORMAL, "What name or jid are you looking for?");
      return;
    }
    scr_RosterSearch(string);
    update_roster = TRUE;
  } else if (!strcasecmp(arg, "up")) {
    scr_RosterUp();
  } else if (!strcasecmp(arg, "down")) {
    scr_RosterDown();
  } else
    scr_LogPrint(LPRINT_NORMAL, "Unrecognized parameter!");
}

//  setstatus(recipient, arg)
// Set your Jabber status.
// - if recipient is not NULL, the status is sent to this contact only
// - arg must be "status message" (message is optional)
static void setstatus(const char *recipient, const char *arg)
{
  char **paramlst;
  char *status;
  char *msg;
  enum imstatus st;

  if (!jb_getonline()) {
    scr_LogPrint(LPRINT_NORMAL, "You are not connected");
    return;
  }

  paramlst = split_arg(arg, 2, 0); // status, message
  status = *paramlst;
  msg = *(paramlst+1);

  if (!status) {
    free_arg_lst(paramlst);
    return;
  }

  if      (!strcasecmp(status, "offline"))   st = offline;
  else if (!strcasecmp(status, "online"))    st = available;
  else if (!strcasecmp(status, "avail"))     st = available;
  else if (!strcasecmp(status, "away"))      st = away;
  else if (!strcasecmp(status, "invisible")) st = invisible;
  else if (!strcasecmp(status, "dnd"))       st = dontdisturb;
  else if (!strcasecmp(status, "notavail"))  st = notavail;
  else if (!strcasecmp(status, "free"))      st = freeforchat;
  else {
    scr_LogPrint(LPRINT_NORMAL, "Unrecognized status!");
    free_arg_lst(paramlst);
    return;
  }

  // Use provided message, unless requested status is "invisible"
  if (msg && st != invisible) {
    if (!*msg) msg = NULL;
  } else
    msg = NULL;

  // If a recipient is specified, let's don't use default status messages
  if (recipient && !msg)
    msg = "";

  jb_setstatus(st, recipient, msg);

  free_arg_lst(paramlst);
}

static void do_status(char *arg)
{
  if (!*arg) {
    const char *sm = jb_getstatusmsg();
    scr_LogPrint(LPRINT_NORMAL, "Your status is: [%c] %s",
                 imstatus2char[jb_getstatus()],
                 (sm ? sm : ""));
    return;
  }
  setstatus(NULL, arg);
}

static void do_status_to(char *arg)
{
  char **paramlst;
  char *jid, *st, *msg;

  paramlst = split_arg(arg, 3, 1); // jid, status, [message]
  jid = *paramlst;
  st = *(paramlst+1);
  msg = *(paramlst+2);

  if (!jid || !st) {
    scr_LogPrint(LPRINT_NORMAL, "Wrong usage");
    free_arg_lst(paramlst);
    return;
  }

  // Allow things like /status_to "" away
  if (!*jid || !strcmp(jid, "."))
    jid = NULL;

  if (jid) {
    // The JID has been specified.  Quick check...
    if (check_jid_syntax(jid)) {
      scr_LogPrint(LPRINT_NORMAL, "<%s> is not a valid Jabber id", jid);
      jid = NULL;
    } else {
      mc_strtolower(jid);
    }
  } else {
    // Add the current buddy
    if (current_buddy)
      jid = (char*)buddy_getjid(BUDDATA(current_buddy));
    if (!jid)
      scr_LogPrint(LPRINT_NORMAL, "Please specify a Jabber id");
  }

  if (jid) {
    char *cmd;
    if (!msg)
      msg = "";
    mc_strtolower(jid);
    cmd = g_strdup_printf("%s %s", st, msg);
    scr_LogPrint(LPRINT_LOGNORM, "Sending to <%s> /status %s", jid, cmd);
    setstatus(jid, cmd);
    g_free(cmd);
  }
  free_arg_lst(paramlst);
}

static void do_add(char *arg)
{
  char **paramlst;
  char *id, *nick;

  if (!jb_getonline()) {
    scr_LogPrint(LPRINT_NORMAL, "You are not connected");
    return;
  }

  paramlst = split_arg(arg, 2, 0); // jid, [nickname]
  id = *paramlst;
  nick = *(paramlst+1);

  if (!id)
    nick = NULL; // Allow things like: /add "" nick
  else if (!*id || !strcmp(id, "."))
    id = NULL;

  if (id) {
    // The JID has been specified.  Quick check...
    if (check_jid_syntax(id)) {
      scr_LogPrint(LPRINT_NORMAL, "<%s> is not a valid Jabber id", id);
      id = NULL;
    } else {
      mc_strtolower(id);
    }
  } else {
    // Add the current buddy
    if (current_buddy)
      id = (char*)buddy_getjid(BUDDATA(current_buddy));
    if (!id)
      scr_LogPrint(LPRINT_NORMAL, "Please specify a Jabber id");
  }

  if (id) {
    // 2nd parameter = optional nickname
    jb_addbuddy(id, nick, NULL);
    scr_LogPrint(LPRINT_LOGNORM, "Sent presence notification request to <%s>",
                 id);
  }
  free_arg_lst(paramlst);
}

static void do_del(char *arg)
{
  const char *jid;

  if (*arg) {
    scr_LogPrint(LPRINT_NORMAL, "Wrong usage");
    return;
  }

  if (!current_buddy) return;
  jid = buddy_getjid(BUDDATA(current_buddy));
  if (!jid) return;

  if (buddy_gettype(BUDDATA(current_buddy)) & ROSTER_TYPE_ROOM) {
    // This is a chatroom
    // If there are resources, we haven't left
    if (buddy_isresource(BUDDATA(current_buddy))) {
      scr_LogPrint(LPRINT_NORMAL, "You haven't left this room!");
      return;
    }
  }

  scr_LogPrint(LPRINT_LOGNORM, "Removing <%s>...", jid);
  jb_delbuddy(jid);
  scr_ShowBuddyWindow();
}

static void do_group(char *arg)
{
  gpointer group;
  guint leave_windowbuddy;

  if (!*arg) {
    scr_LogPrint(LPRINT_NORMAL, "Missing parameter");
    return;
  }

  if (!current_buddy) return;

  group = buddy_getgroup(BUDDATA(current_buddy));
  // We'll have to redraw the chat window if we're not currently on the group
  // entry itself, because it means we'll have to leave the current buddy
  // chat window.
  leave_windowbuddy = (group != BUDDATA(current_buddy));

  if (!(buddy_gettype(group) & ROSTER_TYPE_GROUP)) {
    scr_LogPrint(LPRINT_NORMAL, "You need to select a group");
    return;
  }

  if (!strcasecmp(arg, "expand") || !strcasecmp(arg, "unfold")) {
    buddy_setflags(group, ROSTER_FLAG_HIDE, FALSE);
  } else if (!strcasecmp(arg, "shrink") || !strcasecmp(arg, "fold")) {
    buddy_setflags(group, ROSTER_FLAG_HIDE, TRUE);
  } else if (!strcasecmp(arg, "toggle")) {
    buddy_setflags(group, ROSTER_FLAG_HIDE,
            !(buddy_getflags(group) & ROSTER_FLAG_HIDE));
  } else {
    scr_LogPrint(LPRINT_NORMAL, "Unrecognized parameter!");
    return;
  }

  buddylist_build();
  update_roster = TRUE;
  if (leave_windowbuddy) scr_ShowBuddyWindow();
}

static int send_message_to(const char *jid, const char *msg)
{
  char *bare_jid, *rp;

  if (!jid || !*jid) {
    scr_LogPrint(LPRINT_NORMAL, "JID is missing");
    return 1;
  }
  if (!msg || !*msg) {
    scr_LogPrint(LPRINT_NORMAL, "Message is missing");
    return 1;
  }
  if (check_jid_syntax((char*)jid)) {
    scr_LogPrint(LPRINT_NORMAL, "<%s> is not a valid Jabber id", jid);
    return 1;
  }

  // We must use the bare jid in hk_message_out()
  rp = strchr(jid, '/');
  if (rp) bare_jid = g_strndup(jid, rp - jid);
  else   bare_jid = (char*)jid;

  // Jump to window, create one if needed
  scr_RosterJumpJid(bare_jid);

  // Check if we're sending a message to a conference room
  // If not, we must make sure rp is NULL, for hk_message_out()
  if (rp) {
    if (roster_find(bare_jid, jidsearch, ROSTER_TYPE_ROOM)) rp++;
    else rp = NULL;
  }

  // local part (UI, logging, etc.)
  hk_message_out(bare_jid, rp, 0, msg);

  // Network part
  jb_send_msg(jid, msg, ROSTER_TYPE_USER, NULL);

  if (rp) g_free(bare_jid);
  return 0;
}

static void do_say(char *arg)
{
  gpointer bud;

  scr_set_chatmode(TRUE);

  if (!current_buddy) {
    scr_LogPrint(LPRINT_NORMAL, "Who are you talking to??");
    return;
  }

  bud = BUDDATA(current_buddy);
  if (!(buddy_gettype(bud) &
        (ROSTER_TYPE_USER|ROSTER_TYPE_AGENT|ROSTER_TYPE_ROOM))) {
    scr_LogPrint(LPRINT_NORMAL, "This is not a user");
    return;
  }

  buddy_setflags(bud, ROSTER_FLAG_LOCK, TRUE);
  send_message(arg);
}

static void do_msay(char *arg)
{
  /* Parameters: begin verbatim abort send send_to */
  char **paramlst;
  char *subcmd;

  paramlst = split_arg(arg, 2, 1); // subcmd, arg
  subcmd = *paramlst;
  arg = *(paramlst+1);

  if (!subcmd || !*subcmd) {
    scr_LogPrint(LPRINT_NORMAL, "Missing parameter");
    scr_LogPrint(LPRINT_NORMAL, "Please read the manual before using "
                 "the /msay command.");
    scr_LogPrint(LPRINT_NORMAL, "(Use \"/msay begin\" to enter "
                 "multi-line mode...)");
    free_arg_lst(paramlst);
    return;
  }

  if (!strcasecmp(subcmd, "abort")) {
    if (scr_get_multimode())
      scr_LogPrint(LPRINT_NORMAL, "Leaving multi-line message mode");
    scr_set_multimode(FALSE);
    return;
  } else if ((!strcasecmp(subcmd, "begin")) ||
             (!strcasecmp(subcmd, "verbatim"))) {
    if (!strcasecmp(subcmd, "verbatim"))
      scr_set_multimode(2);
    else
      scr_set_multimode(1);

    scr_LogPrint(LPRINT_NORMAL, "Entered multi-line message mode.");
    scr_LogPrint(LPRINT_NORMAL, "Select a buddy and use \"/msay send\" "
                 "when your message is ready.");
    return;
  } else if (strcasecmp(subcmd, "send") && strcasecmp(subcmd, "send_to")) {
    scr_LogPrint(LPRINT_NORMAL, "Unrecognized parameter!");
    return;
  }

  /* send/send_to command */

  if (!scr_get_multimode()) {
    scr_LogPrint(LPRINT_NORMAL, "No message to send.  "
                 "Use \"/msay begin\" first.");
    return;
  }

  scr_set_chatmode(TRUE);

  if (!strcasecmp(subcmd, "send_to")) {
    // Let's send to the specified JID.  We leave now if there
    // has been an error (so we don't leave multi-line mode).
    if (send_message_to(arg, scr_get_multiline()))
      return;
  } else { // Send to currently selected buddy
    gpointer bud;

    if (!current_buddy) {
      scr_LogPrint(LPRINT_NORMAL, "Who are you talking to??");
      return;
    }

    bud = BUDDATA(current_buddy);
    if (!(buddy_gettype(bud) & (ROSTER_TYPE_USER|ROSTER_TYPE_ROOM))) {
      scr_LogPrint(LPRINT_NORMAL, "This is not a user");
      return;
    }

    buddy_setflags(bud, ROSTER_FLAG_LOCK, TRUE);
    send_message(scr_get_multiline());
  }
  scr_set_multimode(FALSE);
}

static void do_say_to(char *arg)
{
  char **paramlst;
  char *jid, *msg;

  if (!jb_getonline()) {
    scr_LogPrint(LPRINT_NORMAL, "You are not connected");
    return;
  }

  paramlst = split_arg(arg, 2, 1); // jid, message
  jid = *paramlst;
  msg = *(paramlst+1);

  if (!jid) {
    scr_LogPrint(LPRINT_NORMAL, "Wrong usage");
    free_arg_lst(paramlst);
    return;
  }

  send_message_to(jid, msg);
  free_arg_lst(paramlst);
}

//  buffer_updown(updown, nblines)
// updown: -1=up, +1=down
inline static void buffer_updown(int updown, char *nlines)
{
  int nblines;

  if (!nlines || !*nlines)
    nblines = 0;
  else
    nblines = atoi(nlines);

  if (nblines >= 0)
    scr_BufferScrollUpDown(updown, nblines);
}

static void buffer_search(int direction, char *arg)
{
  if (!arg || !*arg) {
    scr_LogPrint(LPRINT_NORMAL, "Missing parameter");
    return;
  }

  scr_BufferSearch(direction, arg);
}

static void buffer_date(char *date)
{
  time_t t;

  if (!date || !*date) {
    scr_LogPrint(LPRINT_NORMAL, "Missing parameter");
    return;
  }

  strip_arg_special_chars(date);

  t = from_iso8601(date, 0);
  if (t)
    scr_BufferDate(t);
  else
    scr_LogPrint(LPRINT_NORMAL, "Wrong parameter");
}

static void buffer_percent(char *arg1, char *arg2)
{
  // Basically, user has typed "%arg1 arg2"
  // "%50"  -> arg1 = 50, arg2 null pointer
  // "% 50" -> arg1 = \0, arg2 = 50

  if (!*arg1 && (!arg2 || !*arg2)) { // No value
    scr_LogPrint(LPRINT_NORMAL, "Missing parameter");
    return;
  }

  if (*arg1 && arg2 && *arg2) {     // Two values
    scr_LogPrint(LPRINT_NORMAL, "Wrong parameters");
    return;
  }

  scr_BufferPercent(atoi((*arg1 ? arg1 : arg2)));
}

static void do_buffer(char *arg)
{
  char **paramlst;
  char *subcmd;

  if (!current_buddy) return;

  if (buddy_gettype(BUDDATA(current_buddy)) & ROSTER_TYPE_GROUP) {
    scr_LogPrint(LPRINT_NORMAL, "Groups have no buffer");
    return;
  }

  paramlst = split_arg(arg, 2, 1); // subcmd, arg
  subcmd = *paramlst;
  arg = *(paramlst+1);

  if (!subcmd || !*subcmd) {
    scr_LogPrint(LPRINT_NORMAL, "Missing parameter");
    free_arg_lst(paramlst);
    return;
  }

  if (!strcasecmp(subcmd, "top")) {
    scr_BufferTopBottom(-1);
  } else if (!strcasecmp(subcmd, "bottom")) {
    scr_BufferTopBottom(1);
  } else if (!strcasecmp(subcmd, "clear")) {
    scr_BufferClear();
  } else if (!strcasecmp(subcmd, "up")) {
    buffer_updown(-1, arg);
  } else if (!strcasecmp(subcmd, "down")) {
    buffer_updown(1, arg);
  } else if (!strcasecmp(subcmd, "search_backward")) {
    buffer_search(-1, arg);
  } else if (!strcasecmp(subcmd, "search_forward")) {
    buffer_search(1, arg);
  } else if (!strcasecmp(subcmd, "date")) {
    buffer_date(arg);
  } else if (*subcmd == '%') {
    buffer_percent(subcmd+1, arg);
  } else {
    scr_LogPrint(LPRINT_NORMAL, "Unrecognized parameter!");
  }

  free_arg_lst(paramlst);
}

static void do_clear(char *arg)    // Alias for "/buffer clear"
{
  do_buffer("clear");
}

static void do_info(char *arg)
{
  gpointer bud;
  const char *jid, *name;
  guint type;
  char *buffer;
  enum subscr esub;

  if (!current_buddy) return;
  bud = BUDDATA(current_buddy);

  jid    = buddy_getjid(bud);
  name   = buddy_getname(bud);
  type   = buddy_gettype(bud);
  esub   = buddy_getsubscription(bud);

  buffer = g_new(char, 4096);

  if (jid) {
    GSList *resources;
    char *bstr = "unknown";

    // Enter chat mode
    scr_set_chatmode(TRUE);
    scr_ShowBuddyWindow();

    snprintf(buffer, 4095, "jid:  <%s>", jid);
    scr_WriteIncomingMessage(jid, buffer, 0, HBB_PREFIX_INFO);
    if (name) {
      snprintf(buffer, 4095, "Name: %s", name);
      scr_WriteIncomingMessage(jid, buffer, 0, HBB_PREFIX_INFO);
    }

    if (type == ROSTER_TYPE_USER)       bstr = "user";
    else if (type == ROSTER_TYPE_ROOM)  bstr = "chatroom";
    else if (type == ROSTER_TYPE_AGENT) bstr = "agent";
    snprintf(buffer, 127, "Type: %s", bstr);
    scr_WriteIncomingMessage(jid, buffer, 0, HBB_PREFIX_INFO);

    if (esub == sub_both)     bstr = "both";
    else if (esub & sub_from) bstr = "from";
    else if (esub & sub_to)   bstr = "to";
    else bstr = "none";
    snprintf(buffer, 64, "Subscription: %s", bstr);
    if (esub & sub_pending)
      strcat(buffer, " (pending)");
    scr_WriteIncomingMessage(jid, buffer, 0, HBB_PREFIX_INFO);

    resources = buddy_getresources(bud);
    for ( ; resources ; resources = g_slist_next(resources) ) {
      gchar rprio;
      enum imstatus rstatus;
      const char *rst_msg;
      time_t rst_time;

      rprio   = buddy_getresourceprio(bud, resources->data);
      rstatus = buddy_getstatus(bud, resources->data);
      rst_msg = buddy_getstatusmsg(bud, resources->data);
      rst_time = buddy_getstatustime(bud, resources->data);

      snprintf(buffer, 4095, "Resource: [%c] (%d) %s", imstatus2char[rstatus],
               rprio, (char*)resources->data);
      scr_WriteIncomingMessage(jid, buffer, 0, HBB_PREFIX_INFO);
      if (rst_msg) {
        snprintf(buffer, 4095, "Status message: %s", rst_msg);
        scr_WriteIncomingMessage(jid, buffer, 0, HBB_PREFIX_INFO);
      }
      if (rst_time) {
        char tbuf[128];

        strftime(tbuf, sizeof(tbuf), "%Y-%m-%d %H:%M:%S", localtime(&rst_time));
        snprintf(buffer, 127, "Status timestamp: %s", tbuf);
        scr_WriteIncomingMessage(jid, buffer, 0, HBB_PREFIX_INFO);
      }
    }
  } else {
    if (name) scr_LogPrint(LPRINT_NORMAL, "Name: %s", name);
    scr_LogPrint(LPRINT_NORMAL, "Type: %s",
                 ((type == ROSTER_TYPE_GROUP) ? "group" : "unknown"));
  }

  g_free(buffer);
}

// room_names() is a variation of do_info(), for chatrooms only
static void room_names(gpointer bud, char *arg)
{
  const char *jid;
  char *buffer;
  GSList *resources;

  if (*arg) {
    scr_LogPrint(LPRINT_NORMAL, "Unknown parameter");
    return;
  }

  // Enter chat mode
  scr_set_chatmode(TRUE);
  scr_ShowBuddyWindow();

  jid    = buddy_getjid(bud);

  buffer = g_new(char, 4096);
  strncpy(buffer, "Room members:", 127);
  scr_WriteIncomingMessage(jid, buffer, 0, HBB_PREFIX_INFO);

  resources = buddy_getresources(bud);
  for ( ; resources ; resources = g_slist_next(resources) ) {
    enum imstatus rstatus;
    const char *rst_msg;

    rstatus = buddy_getstatus(bud, resources->data);
    rst_msg = buddy_getstatusmsg(bud, resources->data);

    snprintf(buffer, 4095, "[%c] %s", imstatus2char[rstatus],
             (char*)resources->data);
    scr_WriteIncomingMessage(jid, buffer, 0, HBB_PREFIX_INFO);
    if (rst_msg) {
      snprintf(buffer, 4095, "Status message: %s", rst_msg);
      scr_WriteIncomingMessage(jid, buffer, 0, HBB_PREFIX_INFO);
    }
  }

  g_free(buffer);
}

static void do_rename(char *arg)
{
  gpointer bud;
  const char *jid, *group;
  guint type;
  char *newname, *p;

  if (!*arg) {
    scr_LogPrint(LPRINT_NORMAL, "Missing parameter");
    return;
  }

  if (!current_buddy) return;
  bud = BUDDATA(current_buddy);

  jid   = buddy_getjid(bud);
  group = buddy_getgroupname(bud);
  type  = buddy_gettype(bud);

  if (type & ROSTER_TYPE_GROUP) {
    scr_LogPrint(LPRINT_NORMAL, "You can't rename groups");
    return;
  }

  newname = g_strdup(arg);
  // Remove trailing space
  for (p = newname; *p; p++) ;
  while (p > newname && *p == ' ') *p = 0;

  strip_arg_special_chars(newname);

  buddy_setname(bud, newname);
  jb_updatebuddy(jid, newname, group);

  g_free(newname);
  update_roster = TRUE;
}

static void do_move(char *arg)
{
  gpointer bud;
  const char *jid, *name, *oldgroupname;
  guint type;
  char *newgroupname, *p;

  if (!current_buddy) return;
  bud = BUDDATA(current_buddy);

  jid  = buddy_getjid(bud);
  name = buddy_getname(bud);
  type = buddy_gettype(bud);

  oldgroupname = buddy_getgroupname(bud);

  if (type & ROSTER_TYPE_GROUP) {
    scr_LogPrint(LPRINT_NORMAL, "You can't move groups!");
    return;
  }

  newgroupname = g_strdup(arg);
  // Remove trailing space
  for (p = newgroupname; *p; p++) ;
  while (p > newgroupname && *p == ' ') *p-- = 0;

  strip_arg_special_chars(newgroupname);

  if (strcmp(oldgroupname, newgroupname)) {
    jb_updatebuddy(jid, name, *newgroupname ? newgroupname : NULL);
    scr_RosterUp();
    buddy_setgroup(bud, newgroupname);
  }

  g_free(newgroupname);
  update_roster = TRUE;
}

static void do_set(char *arg)
{
  guint assign;
  const gchar *option, *value;

  assign = parse_assigment(arg, &option, &value);
  if (!option) {
    scr_LogPrint(LPRINT_NORMAL, "Huh?");
    return;
  }
  if (!assign) {
    // This is a query
    value = settings_opt_get(option);
    if (value) {
      scr_LogPrint(LPRINT_NORMAL, "%s = [%s]", option, value);
    } else
      scr_LogPrint(LPRINT_NORMAL, "Option %s is not set", option);
    return;
  }
  // Update the option
  // XXX Maybe some options should be protected when user is connected
  // (server, username, etc.).  And we should catch some options here, too
  // (hide_offline_buddies for ex.)
  if (!value) {
    settings_del(SETTINGS_TYPE_OPTION, option);
  } else {
    settings_set(SETTINGS_TYPE_OPTION, option, value);
  }
}

static void do_alias(char *arg)
{
  guint assign;
  const gchar *alias, *value;

  assign = parse_assigment(arg, &alias, &value);
  if (!alias) {
    scr_LogPrint(LPRINT_NORMAL, "Huh?");
    return;
  }
  if (!assign) {
    // This is a query
    value = settings_get(SETTINGS_TYPE_ALIAS, alias);
    if (value) {
      scr_LogPrint(LPRINT_NORMAL, "%s = %s", alias, value);
    } else
      scr_LogPrint(LPRINT_NORMAL, "Alias '%s' does not exist", alias);
    return;
  }
  // Check the alias does not conflict with a registered command
  if (cmd_get(alias)) {
      scr_LogPrint(LPRINT_NORMAL, "'%s' is a reserved word!", alias);
      return;
  }
  // Update the alias
  if (!value) {
    if (settings_get(SETTINGS_TYPE_ALIAS, alias)) {
      settings_del(SETTINGS_TYPE_ALIAS, alias);
      // Remove alias from the completion list
      compl_del_category_word(COMPL_CMD, alias);
    }
  } else {
    // Add alias to the completion list, if not already in
    if (!settings_get(SETTINGS_TYPE_ALIAS, alias))
      compl_add_category_word(COMPL_CMD, alias);
    settings_set(SETTINGS_TYPE_ALIAS, alias, value);
  }
}

static void do_bind(char *arg)
{
  guint assign;
  const gchar *keycode, *value;

  assign = parse_assigment(arg, &keycode, &value);
  if (!keycode) {
    scr_LogPrint(LPRINT_NORMAL, "Huh?");
    return;
  }
  if (!assign) {
    // This is a query
    value = settings_get(SETTINGS_TYPE_BINDING, keycode);
    if (value) {
      scr_LogPrint(LPRINT_NORMAL, "Key %s is bound to: %s", keycode, value);
    } else
      scr_LogPrint(LPRINT_NORMAL, "Key %s is not bound", keycode);
    return;
  }
  // Update the key binding
  if (!value)
    settings_del(SETTINGS_TYPE_BINDING, keycode);
  else
    settings_set(SETTINGS_TYPE_BINDING, keycode, value);
}

static void do_rawxml(char *arg)
{
  char **paramlst;
  char *subcmd;

  if (!jb_getonline()) {
    scr_LogPrint(LPRINT_NORMAL, "You are not connected");
    return;
  }

  paramlst = split_arg(arg, 2, 1); // subcmd, arg
  subcmd = *paramlst;
  arg = *(paramlst+1);

  if (!subcmd || !*subcmd) {
    scr_LogPrint(LPRINT_NORMAL, "Please read the manual page"
                 " before using /rawxml :-)");
    free_arg_lst(paramlst);
    return;
  }

  if (!strcasecmp(subcmd, "send"))  {
    gchar *buffer;

    if (!subcmd || !*subcmd) {
      scr_LogPrint(LPRINT_NORMAL, "Missing parameter");
      free_arg_lst(paramlst);
      return;
    }

    // We don't strip_arg_special_chars() here, because it would be a pain for
    // the user to escape quotes in a XML stream...

    buffer = to_utf8(arg);
    if (buffer) {
      scr_LogPrint(LPRINT_NORMAL, "Sending XML string");
      jb_send_raw(buffer);
      g_free(buffer);
    } else {
      scr_LogPrint(LPRINT_NORMAL, "Conversion error in XML string");
    }
  } else {
    scr_LogPrint(LPRINT_NORMAL, "Unrecognized parameter!");
  }

  free_arg_lst(paramlst);
}

//  check_room_subcommand(arg, param_needed, buddy_must_be_a_room)
// - Check if this is a room, if buddy_must_be_a_room is not null
// - Check there is at least 1 parameter, if param_needed is true
// - Return null if one of the checks fails, or a pointer to the first
//   non-space character.
static char *check_room_subcommand(char *arg, bool param_needed,
                                   gpointer buddy_must_be_a_room)
{
  if (buddy_must_be_a_room &&
      !(buddy_gettype(buddy_must_be_a_room) & ROSTER_TYPE_ROOM)) {
    scr_LogPrint(LPRINT_NORMAL, "This isn't a chatroom");
    return NULL;
  }

  if (param_needed) {
    if (!arg) {
      scr_LogPrint(LPRINT_NORMAL, "Missing parameter");
      return NULL;
    }
  }

  if (arg)
    return arg;
  else
    return "";
}

static void room_join(gpointer bud, char *arg)
{
  char **paramlst;
  char *roomname, *nick;

  paramlst = split_arg(arg, 2, 0); // roomid, nickname
  roomname = *paramlst;
  nick = *(paramlst+1);


  if (!roomname || strchr(roomname, '/')) {
    scr_LogPrint(LPRINT_NORMAL, "Invalid room name");
    free_arg_lst(paramlst);
    return;
  }

  if (!nick || !*nick) {
    scr_LogPrint(LPRINT_NORMAL, "Missing parameter (nickname)");
    free_arg_lst(paramlst);
    return;
  }

  // Note that roomname is part of the array allocated by split_arg(),
  // so we can modify it.
  mc_strtolower(roomname);
  jb_room_join(roomname, nick);

  buddylist_build();
  update_roster = TRUE;
  free_arg_lst(paramlst);
}

static void room_invite(gpointer bud, char *arg)
{
  char **paramlst;
  const gchar *roomname;
  char* jid;

  paramlst = split_arg(arg, 2, 1); // jid, [reason]
  jid = *paramlst;
  arg = *(paramlst+1);
  // An empty reason is no reason...
  if (arg && !*arg)
    arg = NULL;

  if (!jid || !*jid) {
    scr_LogPrint(LPRINT_NORMAL, "Missing or incorrect parameter");
    free_arg_lst(paramlst);
    return;
  }

  roomname = buddy_getjid(bud);
  jb_room_invite(roomname, jid, arg);
  scr_LogPrint(LPRINT_LOGNORM, "Invitation sent to <%s>", jid);
  free_arg_lst(paramlst);
}

static void room_affil(gpointer bud, char *arg)
{
  char **paramlst;
  gchar *jid, *rolename;
  struct role_affil ra;
  const char *roomid = buddy_getjid(bud);

  paramlst = split_arg(arg, 3, 1); // jid, new_affil, [reason]
  jid = *paramlst;
  rolename = *(paramlst+1);
  arg = *(paramlst+2);

  if (!jid || !*jid || !rolename || !*rolename) {
    scr_LogPrint(LPRINT_NORMAL, "Missing parameter");
    free_arg_lst(paramlst);
    return;
  }

  ra.type = type_affil;
  ra.val.affil = affil_none;
  for (; ra.val.affil < imaffiliation_size; ra.val.affil++)
    if (!strcasecmp(rolename, straffil[ra.val.affil]))
      break;

  if (ra.val.affil < imaffiliation_size)
    jb_room_setattrib(roomid, jid, NULL, ra, arg);
  else
    scr_LogPrint(LPRINT_NORMAL, "Wrong affiliation parameter");

  free_arg_lst(paramlst);
}

static void room_role(gpointer bud, char *arg)
{
  char **paramlst;
  gchar *jid, *rolename;
  struct role_affil ra;
  const char *roomid = buddy_getjid(bud);

  paramlst = split_arg(arg, 3, 1); // jid, new_role, [reason]
  jid = *paramlst;
  rolename = *(paramlst+1);
  arg = *(paramlst+2);

  if (!jid || !*jid || !rolename || !*rolename) {
    scr_LogPrint(LPRINT_NORMAL, "Missing parameter");
    free_arg_lst(paramlst);
    return;
  }

  ra.type = type_role;
  ra.val.role = role_none;
  for (; ra.val.role < imrole_size; ra.val.role++)
    if (!strcasecmp(rolename, strrole[ra.val.role]))
      break;

  if (ra.val.role < imrole_size)
    jb_room_setattrib(roomid, jid, NULL, ra, arg);
  else
    scr_LogPrint(LPRINT_NORMAL, "Wrong role parameter");

  free_arg_lst(paramlst);
}


// The expected argument is a Jabber id
static void room_ban(gpointer bud, char *arg)
{
  char **paramlst;
  gchar *jid;
  struct role_affil ra;
  const char *roomid = buddy_getjid(bud);

  paramlst = split_arg(arg, 2, 1); // jid, [reason]
  jid = *paramlst;
  arg = *(paramlst+1);

  if (!jid || !*jid) {
    scr_LogPrint(LPRINT_NORMAL, "Missing parameter (Jabber id)");
    free_arg_lst(paramlst);
    return;
  }

  ra.type = type_affil;
  ra.val.affil = affil_outcast;

  jb_room_setattrib(roomid, jid, NULL, ra, arg);

  free_arg_lst(paramlst);
}

// The expected argument is a nickname
static void room_kick(gpointer bud, char *arg)
{
  char **paramlst;
  gchar *nick;
  struct role_affil ra;
  const char *roomid = buddy_getjid(bud);

  paramlst = split_arg(arg, 2, 1); // nickname, [reason]
  nick = *paramlst;
  arg = *(paramlst+1);

  if (!nick || !*nick) {
    scr_LogPrint(LPRINT_NORMAL, "Missing parameter (nickname)");
    free_arg_lst(paramlst);
    return;
  }

  ra.type = type_role;
  ra.val.affil = role_none;

  jb_room_setattrib(roomid, NULL, nick, ra, arg);

  free_arg_lst(paramlst);
}

static void room_leave(gpointer bud, char *arg)
{
  gchar *roomid, *utf8_nickname;

  utf8_nickname = to_utf8(buddy_getnickname(bud));
  roomid = g_strdup_printf("%s/%s", buddy_getjid(bud), utf8_nickname);
  jb_setstatus(offline, roomid, arg);
  g_free(utf8_nickname);
  g_free(roomid);
}

static void room_nick(gpointer bud, char *arg)
{
  if (!arg || !*arg) {
    const char *nick = buddy_getnickname(bud);
    if (nick)
      scr_LogPrint(LPRINT_NORMAL, "Your nickname is: %s", nick);
    else
      scr_LogPrint(LPRINT_NORMAL, "You have no nickname in this room");
  } else {
    gchar *cmd;
    cmd = g_strdup_printf("%s %s", buddy_getjid(bud), arg);
    room_join(bud, cmd);
    g_free(cmd);
  }
}

static void room_privmsg(gpointer bud, char *arg)
{
  char **paramlst;
  gchar *nick, *cmd;

  paramlst = split_arg(arg, 2, 0); // nickname, message
  nick = *paramlst;
  arg = *(paramlst+1);

  if (!nick || !*nick || !arg || !*arg) {
    scr_LogPrint(LPRINT_NORMAL, "Missing parameter");
    free_arg_lst(paramlst);
    return;
  }

  cmd = g_strdup_printf("%s/%s %s", buddy_getjid(bud), nick, arg);
  do_say_to(cmd);
  g_free(cmd);
  free_arg_lst(paramlst);
}

static void room_remove(gpointer bud, char *arg)
{
  if (*arg) {
    scr_LogPrint(LPRINT_NORMAL, "Unknown parameter");
    return;
  }

  // Quick check: if there are resources, we haven't left
  if (buddy_isresource(bud)) {
    scr_LogPrint(LPRINT_NORMAL, "You haven't left this room!");
    return;
  }
  // Delete the room
  roster_del_user(buddy_getjid(bud));
  scr_ShowBuddyWindow();
  buddylist_build();
  update_roster = TRUE;
}

static void room_topic(gpointer bud, char *arg)
{
  gchar *msg;

  // If no parameter is given, display the current topic
  if (!*arg) {
    const char *topic = buddy_gettopic(bud);
    if (topic)
      scr_LogPrint(LPRINT_NORMAL, "Topic: %s", topic);
    else
      scr_LogPrint(LPRINT_NORMAL, "No topic has been set");
    return;
  }

  // Set the topic
  msg = g_strdup_printf("/me has set the topic to: %s", arg);
  jb_send_msg(buddy_getjid(bud), msg, ROSTER_TYPE_ROOM, arg);
  g_free(msg);
}

static void room_destroy(gpointer bud, char *arg)
{
  gchar *msg;

  if (arg && *arg)
    msg = arg;
  else
    msg = NULL;

  jb_room_destroy(buddy_getjid(bud), NULL, msg);
}

static void room_unlock(gpointer bud, char *arg)
{
  if (*arg) {
    scr_LogPrint(LPRINT_NORMAL, "Unknown parameter");
    return;
  }

  jb_room_unlock(buddy_getjid(bud));
}

static void room_whois(gpointer bud, char *arg)
{
  char **paramlst;
  gchar *nick, *buffer;
  const char *jid, *realjid;
  const char *rst_msg;
  gchar rprio;
  enum imstatus rstatus;
  enum imrole role;
  enum imaffiliation affil;
  time_t rst_time;

  char *strroles[] = { "none", "moderator", "participant", "visitor" };
  char *straffil[] = { "none", "owner", "admin", "member", "outcast" };

  paramlst = split_arg(arg, 1, 0); // nickname
  nick = *paramlst;

  if (!nick || !*nick) {
    scr_LogPrint(LPRINT_NORMAL, "Missing parameter (nickname)");
    free_arg_lst(paramlst);
    return;
  }

  // Enter chat mode
  scr_set_chatmode(TRUE);
  scr_ShowBuddyWindow();

  jid = buddy_getjid(bud);
  rstatus = buddy_getstatus(bud, nick);

  if (rstatus == offline) {
    scr_LogPrint(LPRINT_NORMAL, "No such member: %s", nick);
    free_arg_lst(paramlst);
    return;
  }

  rst_time = buddy_getstatustime(bud, nick);
  rprio   = buddy_getresourceprio(bud, nick);
  rst_msg = buddy_getstatusmsg(bud, nick);
  if (!rst_msg) rst_msg = "";

  role = buddy_getrole(bud, nick);
  affil = buddy_getaffil(bud, nick);
  realjid = buddy_getrjid(bud, nick);

  buffer = g_new(char, 4096);

  snprintf(buffer, 4095, "Whois [%s]", nick);
  scr_WriteIncomingMessage(jid, buffer, 0, HBB_PREFIX_INFO);
  snprintf(buffer, 4095, "Status   : [%c] %s", imstatus2char[rstatus],
           rst_msg);
  scr_WriteIncomingMessage(jid, buffer, 0, HBB_PREFIX_INFO);

  if (rst_time) {
    char tbuf[128];

    strftime(tbuf, sizeof(tbuf), "%Y-%m-%d %H:%M:%S", localtime(&rst_time));
    snprintf(buffer, 127, "Timestamp: %s", tbuf);
    scr_WriteIncomingMessage(jid, buffer, 0, HBB_PREFIX_INFO);
  }

  if (realjid) {
    snprintf(buffer, 4095, "JID      : <%s>", realjid);
    scr_WriteIncomingMessage(jid, buffer, 0, HBB_PREFIX_INFO);
  }

  snprintf(buffer, 4095, "Role     : %s", strroles[role]);
  scr_WriteIncomingMessage(jid, buffer, 0, HBB_PREFIX_INFO);
  snprintf(buffer, 4095, "Affiliat.: %s", straffil[affil]);
  scr_WriteIncomingMessage(jid, buffer, 0, HBB_PREFIX_INFO);
  snprintf(buffer, 4095, "Priority : %d", rprio);
  scr_WriteIncomingMessage(jid, buffer, 0, HBB_PREFIX_INFO);

  scr_WriteIncomingMessage(jid, "End of WHOIS", 0, HBB_PREFIX_INFO);

  g_free(buffer);
  free_arg_lst(paramlst);
}

static void do_room(char *arg)
{
  char **paramlst;
  char *subcmd;
  gpointer bud;

  if (!jb_getonline()) {
    scr_LogPrint(LPRINT_NORMAL, "You are not connected");
    return;
  }

  if (!current_buddy) return;
  bud = BUDDATA(current_buddy);

  paramlst = split_arg(arg, 2, 1); // subcmd, arg
  subcmd = *paramlst;
  arg = *(paramlst+1);

  if (!subcmd || !*subcmd) {
    scr_LogPrint(LPRINT_NORMAL, "Missing parameter");
    free_arg_lst(paramlst);
    return;
  }

  if (!strcasecmp(subcmd, "join"))  {
    if ((arg = check_room_subcommand(arg, TRUE, NULL)) != NULL)
      room_join(bud, arg);
  } else if (!strcasecmp(subcmd, "invite"))  {
    if ((arg = check_room_subcommand(arg, TRUE, bud)) != NULL)
      room_invite(bud, arg);
  } else if (!strcasecmp(subcmd, "affil"))  {
    if ((arg = check_room_subcommand(arg, TRUE, bud)) != NULL)
      room_affil(bud, arg);
  } else if (!strcasecmp(subcmd, "role"))  {
    if ((arg = check_room_subcommand(arg, TRUE, bud)) != NULL)
      room_role(bud, arg);
  } else if (!strcasecmp(subcmd, "ban"))  {
    if ((arg = check_room_subcommand(arg, TRUE, bud)) != NULL)
      room_ban(bud, arg);
  } else if (!strcasecmp(subcmd, "kick"))  {
    if ((arg = check_room_subcommand(arg, TRUE, bud)) != NULL)
      room_kick(bud, arg);
  } else if (!strcasecmp(subcmd, "leave"))  {
    if ((arg = check_room_subcommand(arg, FALSE, bud)) != NULL)
      room_leave(bud, arg);
  } else if (!strcasecmp(subcmd, "names"))  {
    if ((arg = check_room_subcommand(arg, FALSE, bud)) != NULL)
      room_names(bud, arg);
  } else if (!strcasecmp(subcmd, "nick"))  {
    if ((arg = check_room_subcommand(arg, FALSE, bud)) != NULL)
      room_nick(bud, arg);
  } else if (!strcasecmp(subcmd, "privmsg"))  {
    if ((arg = check_room_subcommand(arg, TRUE, bud)) != NULL)
      room_privmsg(bud, arg);
  } else if (!strcasecmp(subcmd, "remove"))  {
    if ((arg = check_room_subcommand(arg, FALSE, bud)) != NULL)
      room_remove(bud, arg);
  } else if (!strcasecmp(subcmd, "destroy"))  {
    if ((arg = check_room_subcommand(arg, FALSE, bud)) != NULL)
      room_destroy(bud, arg);
  } else if (!strcasecmp(subcmd, "unlock"))  {
    if ((arg = check_room_subcommand(arg, FALSE, bud)) != NULL)
      room_unlock(bud, arg);
  } else if (!strcasecmp(subcmd, "topic"))  {
    if ((arg = check_room_subcommand(arg, FALSE, bud)) != NULL)
      room_topic(bud, arg);
  } else if (!strcasecmp(subcmd, "whois"))  {
    if ((arg = check_room_subcommand(arg, TRUE, bud)) != NULL)
      room_whois(bud, arg);
  } else {
    scr_LogPrint(LPRINT_NORMAL, "Unrecognized parameter!");
  }

  free_arg_lst(paramlst);
}

static void do_authorization(char *arg)
{
  char **paramlst;
  char *subcmd;

  if (!jb_getonline()) {
    scr_LogPrint(LPRINT_NORMAL, "You are not connected");
    return;
  }

  paramlst = split_arg(arg, 2, 0); // subcmd, [jid]
  subcmd = *paramlst;
  arg = *(paramlst+1);

  if (!subcmd || !*subcmd) {
    scr_LogPrint(LPRINT_NORMAL, "Missing parameter");
    free_arg_lst(paramlst);
    return;
  }

  // Use the provided jid, if it looks valid
  if (arg) {
    if (!*arg) {
      // If no jid is provided, we use the current selected buddy
      arg = NULL;
    } else {
      if (check_jid_syntax(arg)) {
        scr_LogPrint(LPRINT_NORMAL, "<%s> is not a valid Jabber id", arg);
        free_arg_lst(paramlst);
        return;
      }
    }
  }

  if (!arg) {       // Use the current selected buddy's jid
    gpointer bud;
    guint type;

    if (!current_buddy) return;
    bud = BUDDATA(current_buddy);

    arg  = (char*)buddy_getjid(bud);
    type = buddy_gettype(bud);

    if (!(type & (ROSTER_TYPE_USER|ROSTER_TYPE_AGENT))) {
      scr_LogPrint(LPRINT_NORMAL, "Invalid buddy");
      return;
    }
  }

  if (!strcasecmp(subcmd, "allow"))  {
    jb_subscr_send_auth(arg);
    scr_LogPrint(LPRINT_LOGNORM,
                 "<%s> is allowed to receive your presence updates", arg);
  } else if (!strcasecmp(subcmd, "cancel"))  {
    jb_subscr_cancel_auth(arg);
    scr_LogPrint(LPRINT_LOGNORM,
                 "<%s> is no more allowed to receive your presence updates",
                 arg);
  } else if (!strcasecmp(subcmd, "request"))  {
    jb_subscr_request_auth(arg);
    scr_LogPrint(LPRINT_LOGNORM,
                 "Sent presence notification request to <%s>", arg);
  } else {
    scr_LogPrint(LPRINT_NORMAL, "Unrecognized parameter!");
  }

  free_arg_lst(paramlst);
}

static void do_version(char *arg)
{
  scr_LogPrint(LPRINT_NORMAL, "This is mcabber version %s", PACKAGE_VERSION);
}

static void do_connect(char *arg)
{
  mcabber_connect();
}

static void do_disconnect(char *arg)
{
  jb_disconnect();
}

/* vim: set expandtab cindent cinoptions=>2\:2(0:  For Vim users... */
