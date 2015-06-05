#Drush 配置

##怎么样使用这个目录

Drush 默认不会知道这个目录。你需要把下面这段代码放到本地的 drushrc.php 文件里边儿。

```php
// Load a drushrc.php file from the 'drush' folder at the root of the current
// git repository. Customize as desired.
// (Script by grayside; @see: http://grayside.org/node/93)
$repo_dir = drush_get_option('root') ? drush_get_option('root') : getcwd();
$success = drush_shell_exec('cd %s && git rev-parse --show-toplevel 2> ' . drush_bit_bucket(), $repo_dir);
if ($success) {
  $output = drush_shell_exec_output();
  $repo = $output[0];
  $options['config'] = $repo . '/drush/drushrc.php';
  $options['include'] = $repo . '/drush/commands';
  $options['alias-path'] = $repo . '/drush/aliases';
}
```
把上面的代码放到 drushrc.php 文件里面以后，Drush 就会知道要读取我们的自定义的 drushrc.php ，并且会去搜索
commands 与 aliases 目录下面的命令与别名。

###别名 Aliases
aliases 目录里面存储的是为项目单独创建的命令的别名。在这里也可以共享别名，比如 _@example.staging_, _@example.live_, _@example.rc_ etc..

注意在这里不要存储本地特定的别名，因为它们很可能不会在所有的环境下起作用。

###命令 Commands
commands 目录里面放的是 drush 的命令，你可以跟你的团队共享它们。这里可以存储你自定义的 drush 命令。

默认在这里我们放了 __Registry Rebuild__, __Build__ 还有 __Devify 命令。

####重建注册 Registry Rebuild
了解什么是 Registry Rebuild ，你可以查看这个项目的页面 [项目页面](http://drupal.org/project/registry_rebuild)。

>在 Drupal 7 上，有时候你需要重建注册（一些 PHP 类的文件）。有时候你不能用 cache-clear（ Drush 的清空缓存命令） 命令完成这个工作，因为有些类要在
bootstrap 的时候就需要用到。

####构建 Build
build 命令就是一个 drush 命令，它会去调用一些其它的 drush 命令，比如 updatedb，features-revert-all，cache-clear 等等。这个命令可以保证你的部署
正确有效。下面是这个命令做的事：

    drush updatedb
    drush features-revert-all --force
    drush cc all

需要执行上面这些命令，你可以直接使用 _drush build --yes_ 。

####Devify
在部署的时候你需要定期的去拉取 staging 或者 production 的数据库到本地环境上。这需要用到一些命令还有一些要改变的变量，比如禁用缓存还有更新模块，sanitize
emails 与 passwords，删除敏感的变量。 devify 命令会去 禁用/启用 一些模块，还有一些要 删除/重置 的变量，还会去 sanitizes 数据库。

在 drushrc.php 文件里面去设置 command-specific 的设置，比如：

    /**
     * Settings for devify command.
     */
    $command_specific['devify'] = array(
      'enable-modules' => array('devel', 'advanced_help'),
      'disable-modules' => array('varnish', 'memcache_admin'),
      'delete-variables' => array('googleanalytics_account'),
      'reset-variables' => array('site_mail' => 'local@local.com'),
    );

你只需要输入 _drush devify --yes_ 。

翻译：[宁皓网](http://ninghao.net)
