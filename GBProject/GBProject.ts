import { v4 as uuid } from "uuid";
import * as readline from "readline";

// CONSTANTS
const MAX_BLOCKS = 16896;
const BLOCK_SIZE = 512;
const MAX_NUMBER_OF_BLOCK_ADDRESS = 10;

// Types
enum Directory {
  Parent = "..",
  Current = ".",
}

// DirectoryMap is an array of objects that represent the directories.
// Current and Parent are special keys that represent the current directory and the parent directory
type DirectoryMap = {
  [Directory.Current]: string;
  [Directory.Parent]?: string;
  [fileName: string]: string;
};

type BlockContent = null | string;

enum Permissions {
  Read = "read",
  Write = "write",
  Execute = "execute",
}

// Classes
class User {
  name: string;
  id: string;

  constructor(name: string, id: string) {
    this.name = name;
    this.id = id;
  }
}

class Inode {
  id: string;
  user: User;
  permission: string;
  isDirectory: boolean;
  blocks: number[]; // max 10
  indirectBlock: number | null;
  updatedAt: Date;
  accessAt: Date;
  createdAt: Date;

  constructor(
    user: User,
    isDirectory: boolean,
    blocks: number[],
    indirectBlock?: number
  ) {
    this.id = uuid();
    this.user = user;
    this.permission = "700";
    this.isDirectory = isDirectory;
    this.blocks = blocks;
    this.indirectBlock = indirectBlock || null;
    this.updatedAt = new Date();
    this.accessAt = new Date();
    this.createdAt = new Date();
  }
}

class FileSystem {
  inodes: Inode[];
  directoriesArrMap: DirectoryMap[];
  directoryId: string;
  users: User[];
  loggedUser: User;
  blocks: BlockContent[];

  constructor() {
    this.initializeSystem();
  }

  initializeSystem() {
    const adminUser = new User("admin", uuid());
    const rootInode = new Inode(adminUser, true, []);
    this.inodes = [rootInode];
    this.directoriesArrMap = [{ [Directory.Current]: rootInode.id }];
    this.directoryId = rootInode.id;
    this.users = [adminUser];
    this.loggedUser = adminUser;
    this.blocks = Array.from({ length: MAX_BLOCKS }, () => null);
  }

  // Helper functions
  private findFreeBlockIndex() {
    return this.blocks.findIndex((block) => block === null);
  }

  private findCurrentDirectory(inodeId: string) {
    return this.directoriesArrMap.find(
      (directory) => directory[Directory.Current] === inodeId
    );
  }

  private findParentOfCurrentDirectory(inodeId: string) {
    const currentDirectory = this.findCurrentDirectory(inodeId);
    if (!currentDirectory) return;
    const parentDirectoryId = currentDirectory[Directory.Parent];
    return this.directoriesArrMap.find(
      (directory) => directory[Directory.Current] === parentDirectoryId
    );
  }

  private formatDate(date: Date) {
    const time = date.toLocaleTimeString();
    const month = date.getMonth() + 1;
    const day = date.getDate();
    const year = date.getFullYear();
    return `${month}/${day}/${year} ${time}`;
  }

  private hasPermission(inode: Inode, permission: Permissions) {
    const user = this.loggedUser;
    const isOwner = user.id === inode.user.id;
    const isRoot = user.name === "admin";

    if (isRoot) return true;
    if (isOwner) return true;

    const linuxPermissionsMap = {
      [Permissions.Read]: 4,
      [Permissions.Write]: 2,
      [Permissions.Execute]: 1,
    };

    const inodePermission = inode.permission;
    const otherPermission = Number(inodePermission[2]);
    return otherPermission >= linuxPermissionsMap[permission];
  }

  private getContentDividedInBlocks(content: string) {
    const blocks: string[] = [];
    for (let i = 0; i < content.length; i += BLOCK_SIZE) {
      const block = content.slice(i, i + BLOCK_SIZE);
      blocks.push(block);
    }
    return blocks;
  }

  private getUsedBlocks() {
    return this.blocks.filter((block) => block !== null);
  }

  private getTextInodePermissions(inode: Inode) {
    const linuxPermissionsMap = {
      [Permissions.Read]: "r",
      [Permissions.Write]: "w",
      [Permissions.Execute]: "x",
    };
    let permission = "rwx";
    const inodePermission = inode.permission;

    const groupPermission = Number(inodePermission[1]);
    const groupPermissionBinary = groupPermission.toString(2);
    const groupPermissionBinaryArray = groupPermissionBinary.split("");
    const groupPermissionBinaryArrayWithPadding = Array.from(
      { length: 3 - groupPermissionBinaryArray.length },
      () => "0"
    ).concat(groupPermissionBinaryArray);
    const groupPermissionBinaryArrayWithPermissions =
      groupPermissionBinaryArrayWithPadding.map((binary, index) => {
        if (binary === "0") return "-";
        const permission = Object.keys(linuxPermissionsMap)[index];
        return linuxPermissionsMap[permission];
      });
    const groupPermissionString =
      groupPermissionBinaryArrayWithPermissions.join("");
    permission += groupPermissionString;
    const otherPermission = Number(inodePermission[2]);
    const otherPermissionBinary = otherPermission.toString(2);
    const otherPermissionBinaryArray = otherPermissionBinary.split("");
    const otherPermissionBinaryArrayWithPadding = Array.from(
      { length: 3 - otherPermissionBinaryArray.length },
      () => "0"
    ).concat(otherPermissionBinaryArray);

    const otherPermissionBinaryArrayWithPermissions =
      otherPermissionBinaryArrayWithPadding.map((binary, index) => {
        if (binary === "0") return "-";
        const permission = Object.keys(linuxPermissionsMap)[index];
        return linuxPermissionsMap[permission];
      });
    const otherPermissionString =
      otherPermissionBinaryArrayWithPermissions.join("");
    permission += otherPermissionString;
    return permission;
  }

  // Commands
  formatSystem() {
    this.initializeSystem();
  }

  createUser(name: string) {
    const user = new User(name, uuid());
    this.users.push(user);
    return user;
  }

  listUsers() {
    this.users.forEach((user) => console.log(user.name));
  }

  changeUser(name: string) {
    const user = this.users.find((user) => user.name === name);
    if (!user) {
      console.log("User not found");
      return;
    }
    this.loggedUser = user;
  }

  removeUser(name: string) {
    const user = this.users.find((user) => user.name === name);
    if (!user) {
      console.log("User not found");
      return;
    }
    this.users = this.users.filter((user) => user.name !== name);

    // remove from the directories map

    this.directoriesArrMap.forEach((directory) => {
      const directoryNames = Object.keys(directory);
      directoryNames.forEach((dirName) => {
        const inodeId = directory[dirName];
        const inode = this.inodes.find((inode) => inode.id === inodeId);
        if (!inode) return;
        if (inode.user.id === user.id) {
          delete directory[dirName];
        }
      });
    });

    // remove all files and directories that belong to this user and clear the blocks
    this.blocks.map((block) => {
      if (block === null) return;
      const blockInode = this.inodes.find((inode) =>
        inode.blocks.includes(this.blocks.indexOf(block))
      );
      if (!blockInode) return;
      if (blockInode.user.id === user.id) {
        block = null;
      }
    });
    this.inodes = this.inodes.filter((inode) => inode.user.id !== user.id);
  }

  chown(currentUserName: string, newUserName: string, fileName: string) {
    const currentUser = this.users.find(
      (user) => user.name === currentUserName
    );
    const newUser = this.users.find((user) => user.name === newUserName);
    if (!currentUser || !newUser) {
      console.log("User not found");
      return;
    }

    const directory = this.findCurrentDirectory(this.directoryId);
    const inodeId = directory[fileName];

    const inode = this.inodes.find((inode) => inode.id === inodeId);
    if (!inode) {
      console.log("File not found");
      return;
    }

    const hasPermission = this.hasPermission(inode, Permissions.Write);
    if (!hasPermission) {
      console.log("You don't have permission to change this file");
      return;
    }

    inode.user = newUser;
  }

  chmod(name: string, permission: string) {
    const directory = this.findCurrentDirectory(this.directoryId);
    const inodeId = directory[name];

    const inode = this.inodes.find((inode) => inode.id === inodeId);
    if (!inode) {
      console.log("File not found");
      return;
    }

    const hasPermission = this.hasPermission(inode, Permissions.Write);
    if (!hasPermission) {
      console.log("You don't have permission to change this file");
      return;
    }
    this.inodes = this.inodes.map((inode) => {
      if (inode.id !== inodeId) return inode;
      return {
        ...inode,
        updatedAt: new Date(),
        permission,
      };
    });
  }

  mkdir(name: string) {
    const directory = this.findCurrentDirectory(this.directoryId);

    const inode = new Inode(this.loggedUser, true, []);
    this.inodes.push(inode);
    if (!!directory[name]) {
      console.log("Directory or file with this name already exists");
      return;
    }
    directory[name] = inode.id;

    const newDirectory: DirectoryMap = {
      [Directory.Current]: inode.id,
      [Directory.Parent]: this.directoryId,
    };
    this.directoriesArrMap.push(newDirectory);
  }

  removeDirectory(name: string) {
    const directory = this.findCurrentDirectory(this.directoryId);
    const inodeId = directory[name];

    const inode = this.inodes.find((inode) => inode.id === inodeId);
    if (!inode) {
      console.log("Directory not found");
      return;
    }

    if (!inode.isDirectory) {
      console.log("This is not a directory");
      return;
    }

    const hasPermission = this.hasPermission(inode, Permissions.Write);
    if (!hasPermission) {
      console.log("You don't have permission to remove this directory");
      return;
    }

    delete directory[name];
    this.inodes = this.inodes.filter((inode) => inode.id !== inodeId);
    this.directoriesArrMap = this.directoriesArrMap.filter(
      (directory) => directory[Directory.Current] !== inodeId
    );
  }

  touch(name: string) {
    const freeBlockIndex = this.findFreeBlockIndex();

    if (freeBlockIndex === -1) {
      console.log("No free blocks available");
      return;
    }

    const directory = this.findCurrentDirectory(this.directoryId);
    if (!!directory[name]) {
      console.log("Directory or file with this name already exists");
      return;
    }
    this.blocks[freeBlockIndex] = ""; // No content when creating it

    const inode = new Inode(this.loggedUser, false, [freeBlockIndex]);
    this.inodes.push(inode);
    directory[name] = inode.id;
  }

  saveContent(name: string, content: string) {
    const directory = this.findCurrentDirectory(this.directoryId);
    const inodeId = directory[name];
    const currentInode = this.inodes.find((inode) => inode.id === inodeId);
    const hasPermissionToWrite = this.hasPermission(
      currentInode,
      Permissions.Write
    );

    if (!hasPermissionToWrite) {
      console.log("You don't have permission to write this file");
      return;
    }

    if (currentInode.isDirectory) {
      console.log("This is not a file");
      return;
    }

    if (currentInode.blocks.length === MAX_NUMBER_OF_BLOCK_ADDRESS) {
      console.log("No more space available");
      return;
    }

    const freeBlockIndex = this.findFreeBlockIndex();
    if (freeBlockIndex === -1) {
      console.log("No free blocks available");
      return;
    }

    const contentDividedInBlocks = this.getContentDividedInBlocks(content);

    contentDividedInBlocks.forEach((blockData, index) => {
      if (index === 0) {
        const lastInodeBlockIndex =
          currentInode.blocks[currentInode.blocks.length - 1];
        const previousBlockData = this.blocks[lastInodeBlockIndex];
        if (previousBlockData.length === 0) {
          // We can use this block because it's empty
          this.blocks[lastInodeBlockIndex] = blockData;
        } else {
          // We can use the freeBlockIndex because we already checked that there is space available
          this.blocks[freeBlockIndex] = blockData;
          currentInode.blocks.push(freeBlockIndex);
        }
      } else {
        const newFreeBlockIndex = this.findFreeBlockIndex();
        if (newFreeBlockIndex === -1) {
          console.log("No free blocks available");
          return;
        }
        const hasReachedMaxNumberOfBlocks =
          currentInode.blocks.length === MAX_NUMBER_OF_BLOCK_ADDRESS;

        if (this.getUsedBlocks().length === MAX_BLOCKS) {
          console.log("No more space on disk available");
          return;
        }

        if (
          hasReachedMaxNumberOfBlocks &&
          currentInode.indirectBlock !== null
        ) {
          console.log("No more space available");
          return;
        }

        if (
          hasReachedMaxNumberOfBlocks &&
          currentInode.indirectBlock === null
        ) {
          // We can use the freeBlockIndex because we already checked that there is space available
          this.blocks[newFreeBlockIndex] = blockData;
          currentInode.indirectBlock = newFreeBlockIndex;
          return;
        }

        this.blocks[newFreeBlockIndex] = blockData;
        currentInode.blocks.push(newFreeBlockIndex);
      }
    });

    this.inodes = this.inodes.map((inode) => {
      if (inode.id !== currentInode.id) return inode;
      return {
        ...currentInode,
        updatedAt: new Date(),
      };
    });
  }

  cat(name: string) {
    const directory = this.findCurrentDirectory(this.directoryId);
    const inodeId = directory[name];
    const currentInode = this.inodes.find((inode) => inode.id === inodeId);
    const hasPermissionToRead = this.hasPermission(
      currentInode,
      Permissions.Read
    );
    if (!hasPermissionToRead) {
      console.log("You don't have permission to read this file");
      return;
    }

    if (currentInode.isDirectory) {
      console.log("This is not a file");
      return;
    }

    this.inodes = this.inodes.map((inode) => {
      if (inode.id !== currentInode.id) return inode;
      return {
        ...currentInode,
        accessAt: new Date(),
      };
    });

    let content = "";
    content = currentInode.blocks
      .map((blockIndex) => this.blocks[blockIndex])
      .join(" ");
    if (!!currentInode.indirectBlock) {
      const indirectBlockContent = this.blocks[currentInode.indirectBlock];
      if (indirectBlockContent) {
        content += indirectBlockContent;
      }
    }
    console.log(content);
  }

  removeFile(name: string) {
    const directory = this.findCurrentDirectory(this.directoryId);
    const inodeId = directory[name];

    const inode = this.inodes.find((inode) => inode.id === inodeId);
    if (!inode) {
      console.log("File not found");
      return;
    }

    const hasPermission = this.hasPermission(inode, Permissions.Write);
    if (!hasPermission) {
      console.log("You don't have permission to remove this file");
      return;
    }

    for (let i = 0; i < inode.blocks.length; i++) {
      const blockIndex = inode.blocks[i];
      this.blocks[blockIndex] = null;
    }

    delete directory[name];
    this.inodes = this.inodes.filter((inode) => inode.id !== inodeId);
  }

  cd(name: string) {
    const currentDirectory = this.findCurrentDirectory(this.directoryId);
    const currentInode = this.inodes.find(
      (inode) => inode.id === currentDirectory[name]
    );

    if (!currentInode) {
      console.log("Directory not found");
      return;
    }

    if (!currentInode.isDirectory) {
      console.log("This is not a directory");
      return;
    }

    const hasPermission = this.hasPermission(currentInode, Permissions.Read);
    if (!hasPermission) {
      console.log("You don't have permission to access this directory");
      return;
    }

    this.inodes = this.inodes.map((inode) => {
      if (inode.id !== currentInode.id) return inode;
      return {
        ...currentInode,
        accessAt: new Date(),
      };
    });

    if (name === "..") {
      const parentDirectoryId = currentDirectory[Directory.Parent];
      if (!parentDirectoryId) {
        console.log("You are in the root directory");
        return;
      }
      this.directoryId = parentDirectoryId;
    } else {
      this.directoryId = currentDirectory[name];
    }
  }

  getCurrentDirectory() {
    const currentDirectory = this.findCurrentDirectory(this.directoryId);
    const parentDirectory = this.findParentOfCurrentDirectory(this.directoryId);
    if (!parentDirectory) return "~";
    return Object.keys(parentDirectory).find(
      (key) => parentDirectory[key] === currentDirectory[Directory.Current]
    );
  }

  ls(param?: string) {
    // Going to look for all directories and files inside the current directory
    const directory = this.findCurrentDirectory(this.directoryId);
    if (!directory) {
      return;
    }

    const directoryNames = Object.keys(directory);
    const filteredDirectoryNames = directoryNames.filter(
      (dirName) => dirName !== Directory.Current && dirName !== Directory.Parent
    );

    const mappedDirectoryNames = filteredDirectoryNames.map((dirName) => {
      const inodeId = directory[dirName];
      const inode = this.inodes.find((inode) => inode.id === inodeId);
      if (!inode) return;
      const isDirectory = inode.isDirectory;
      const updatedAt = this.formatDate(inode.updatedAt);
      const accessAt = this.formatDate(inode.accessAt);
      const createdAt = this.formatDate(inode.createdAt);
      const type = isDirectory ? "d" : "f";
      const textPermission = this.getTextInodePermissions(inode);
      const permission = `${type}${textPermission}`;
      const isUsingIndirectBlock = !!inode.indirectBlock;
      const owner = inode.user.name;
      const blocks = (() => {
        if (isDirectory) return "-";
        return isUsingIndirectBlock
          ? inode.blocks.length + 1
          : inode.blocks.length;
      })();

      return {
        name: dirName,
        permission,
        owner,
        blocks,
        access_at: accessAt,
        created_at: createdAt,
        updated_at: updatedAt,
      };
    });

    if (!param) {
      return mappedDirectoryNames.forEach((dir) => console.log(dir.name));
    }

    if (param === "-l") {
      if (mappedDirectoryNames.length === 0) return;
      const reducedDirectoryNames = mappedDirectoryNames.reduce(
        (acc, { name, ...curr }) => {
          if (!curr) return acc;
          return { ...acc, [name]: curr };
        },
        {}
      );

      console.table(reducedDirectoryNames);
      return;
    }
    console.log("Command not found");
  }
}

const fileSystem = new FileSystem();

const rl = readline.createInterface({
  input: process.stdin,
  output: process.stdout,
});

const ask = (question: string) =>
  new Promise<string>((resolve) => rl.question(question, resolve));

const main = async () => {
  const currentUser = fileSystem.loggedUser.name;
  const currentDirectoryName = fileSystem.getCurrentDirectory();

  const command = await ask(`${currentUser} ${currentDirectoryName} % `);
  const [commandName, ...args] = command.split(" ");
  switch (commandName) {
    case "format":
      fileSystem.formatSystem();
      break;
    case "adduser":
      fileSystem.createUser(args[0]);
      break;
    case "lsuser":
      fileSystem.listUsers();
      break;
    case "changeuser":
      fileSystem.changeUser(args[0]);
      break;
    case "rmuser":
      fileSystem.removeUser(args[0]);
      break;
    case "chown":
      fileSystem.chown(args[0], args[1], args[2]);
      break;
    case "chmod":
      fileSystem.chmod(args[0], args[1]);
      break;
    case "mkdir":
      fileSystem.mkdir(args[0]);
      break;
    case "rmdir":
      fileSystem.removeDirectory(args[0]);
      break;
    case "touch":
      fileSystem.touch(args[0]);
      break;
    case "write":
      const content = args.filter((_, index) => index !== 0).join(" ");
      fileSystem.saveContent(args[0], content);
      break;
    case "cat":
      fileSystem.cat(args[0]);
      break;
    case "rm":
      fileSystem.removeFile(args[0]);
      break;
    case "cd":
      fileSystem.cd(args[0]);
      break;
    case "ls":
      fileSystem.ls(args[0]);
      break;

    case "clear":
      console.clear();
      break;
    case "exit":
      rl.close();
      break;
    default:
      console.log("Command not found");
      break;
  }
  main();
};
main();
